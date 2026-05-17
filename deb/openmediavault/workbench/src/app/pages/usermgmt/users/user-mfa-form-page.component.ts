/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import { finalize } from 'rxjs/operators';

import {
  FormPageButtonConfig,
  FormPageConfig
} from '~/app/core/components/intuition/models/form-page-config.type';
import { translate } from '~/app/i18n.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

/**
 * Page component for managing TOTP two-factor authentication for the
 * currently logged-in user. Accessible at /usermgmt/mfa.
 *
 * The page uses the OMV declarative form framework exclusively. Each
 * state of the MFA setup flow has its own FormPageConfig; the active
 * config is selected by the `state` property and rendered via
 * <omv-intuition-form-page *ngIf="state === '...'">. Because *ngIf
 * destroys and recreates the form component on every state transition,
 * config field values can be safely set immediately before changing the
 * state without worrying about stale form controls.
 *
 * State machine:
 *  loading   → initial RPC call to check current MFA status
 *  disabled  → MFA is off; user can start the setup flow
 *  setup     → QR code is displayed; awaiting confirmation code
 *  enabled   → MFA is active; user can disable it
 *
 * Security notes:
 *  - The provisional secret from generateTotpSecretByContext is never
 *    stored until the user confirms it with a valid TOTP code.
 *  - Disabling MFA requires the current TOTP code, preventing a stolen
 *    browser session from silently removing MFA protection.
 */
@Component({
  selector: 'omv-user-mfa-form-page',
  templateUrl: './user-mfa-form-page.component.html'
})
export class UserMfaFormPageComponent implements OnInit {
  public state: 'loading' | 'disabled' | 'setup' | 'enabled' = 'loading';

  /** The provisional secret held in memory during the setup flow only. */
  private provisionalSecret = '';

  // -----------------------------------------------------------------
  // Form configs — one per visible state.
  // -----------------------------------------------------------------

  public disabledConfig: FormPageConfig = {
    fields: [
      {
        type: 'hint',
        hintType: 'info',
        text: gettext(
          'Two-factor authentication is currently disabled. ' +
          'Enable it to add an extra layer of security to your account.'
        )
      }
    ],
    buttons: [
      {
        template: 'submit',
        text: gettext('Enable Two-Factor Authentication'),
        execute: {
          type: 'click',
          click: this.onStartSetup.bind(this)
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'click',
          click: this.onCancel.bind(this)
        }
      }
    ]
  };

  /**
   * Config for the setup state.
   * The `image` and `secret` field values are filled in by onStartSetup()
   * before state switches to 'setup', so the newly created form always
   * receives the correct initial values.
   */
  public setupConfig: FormPageConfig = {
    fields: [
      {
        type: 'paragraph',
        text: gettext(
          'Scan the QR code below with your authenticator app ' +
          '(Google Authenticator, Authy, etc.). ' +
          'If your app does not support QR codes, enter the secret key manually.'
        )
      },
      {
        type: 'image',
        name: 'qrcode',
        label: gettext('TOTP QR code'),
        value: '',
        submitValue: false,
        width: 256,
        height: 256
      },
      {
        type: 'textInput',
        name: 'secret',
        label: gettext('Secret key (manual entry)'),
        readonly: true,
        value: '',
        submitValue: false
      },
      {
        type: 'paragraph',
        text: gettext(
          'Once your authenticator app is configured, enter the ' +
          '6-digit code it shows to confirm the setup.'
        )
      },
      {
        type: 'textInput',
        name: 'code',
        label: gettext('Verification code'),
        autocomplete: 'one-time-code',
        value: '',
        validators: {
          required: true,
          minLength: 6,
          maxLength: 6,
          pattern: {
            pattern: '^[0-9]{6}$',
            errorData: gettext('Must be a 6-digit number.')
          }
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        text: gettext('Verify and Enable'),
        execute: {
          type: 'click',
          click: this.onConfirmSetup.bind(this)
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'click',
          click: this.onCancelSetup.bind(this)
        }
      }
    ]
  };

  public enabledConfig: FormPageConfig = {
    fields: [
      {
        type: 'hint',
        hintType: 'info',
        text: gettext(
          'Two-factor authentication is enabled. ' +
          'To disable it, enter the current 6-digit code from your authenticator app.'
        )
      },
      {
        type: 'textInput',
        name: 'code',
        label: gettext('Current verification code'),
        autocomplete: 'one-time-code',
        value: '',
        validators: {
          required: true,
          minLength: 6,
          maxLength: 6,
          pattern: {
            pattern: '^[0-9]{6}$',
            errorData: gettext('Must be a 6-digit number.')
          }
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        text: gettext('Disable Two-Factor Authentication'),
        execute: {
          type: 'click',
          click: this.onDisableMfa.bind(this)
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'click',
          click: this.onCancel.bind(this)
        }
      }
    ]
  };

  constructor(
    private blockUiService: BlockUiService,
    private notificationService: NotificationService,
    private rpcService: RpcService,
    private router: Router
  ) {}

  ngOnInit(): void {
    this.loadStatus();
  }

  /** Fetch the current TOTP status for the logged-in user. */
  loadStatus(): void {
    this.state = 'loading';
    this.rpcService.request('UserMgmt', 'getTotpStatusByContext').subscribe({
      next: (res: { enabled: boolean }) => {
        this.state = res.enabled ? 'enabled' : 'disabled';
      },
      error: () => {
        // On error fall back to the disabled view so the page is never stuck loading.
        this.state = 'disabled';
      }
    });
  }

  /**
   * Start the MFA setup flow: ask the backend to generate a fresh TOTP
   * secret and QR code, then transition to the 'setup' state.
   */
  onStartSetup(_buttonConfig: FormPageButtonConfig, _values: Record<string, any>): void {
    this.blockUiService.start(translate(gettext('Generating secret...')));
    this.rpcService
      .request('UserMgmt', 'generateTotpSecretByContext')
      .pipe(finalize(() => this.blockUiService.stop()))
      .subscribe({
        next: (res: { secret: string; uri: string; qrDataUrl: string }) => {
          this.provisionalSecret = res.secret;
          // Patch field values before switching state. Because the
          // template uses *ngIf, the form component will be created fresh
          // and will read these values during its own ngOnInit.
          const fields = this.setupConfig.fields;
          const qrField = fields.find((f) => f.name === 'qrcode');
          if (qrField) qrField.value = res.qrDataUrl;
          const secretField = fields.find((f) => f.name === 'secret');
          if (secretField) secretField.value = res.secret;
          this.state = 'setup';
        }
      });
  }

  /**
   * Confirm setup: submit the provisional secret and the TOTP code the
   * user scanned from the QR image. The backend verifies before storing.
   */
  onConfirmSetup(_buttonConfig: FormPageButtonConfig, values: Record<string, any>): void {
    this.blockUiService.start(translate(gettext('Enabling two-factor authentication...')));
    this.rpcService
      .request('UserMgmt', 'enableTotpByContext', {
        code: String(values['code']).trim(),
        secret: this.provisionalSecret
      })
      .pipe(finalize(() => this.blockUiService.stop()))
      .subscribe({
        next: () => {
          this.notificationService.show(
            NotificationType.success,
            translate(gettext('Two-factor authentication has been enabled.'))
          );
          this.provisionalSecret = '';
          this.state = 'enabled';
        }
      });
  }

  /** Cancel setup and return to the disabled view without saving anything. */
  onCancelSetup(_buttonConfig: FormPageButtonConfig, _values: Record<string, any>): void {
    this.provisionalSecret = '';
    this.state = 'disabled';
  }

  /**
   * Disable TOTP MFA after the user confirms with their current TOTP code.
   * Requiring the code prevents a stolen session from silently removing MFA.
   */
  onDisableMfa(_buttonConfig: FormPageButtonConfig, values: Record<string, any>): void {
    this.blockUiService.start(translate(gettext('Disabling two-factor authentication...')));
    this.rpcService
      .request('UserMgmt', 'disableTotpByContext', {
        code: String(values['code']).trim()
      })
      .pipe(finalize(() => this.blockUiService.stop()))
      .subscribe({
        next: () => {
          this.notificationService.show(
            NotificationType.success,
            translate(gettext('Two-factor authentication has been disabled.'))
          );
          this.state = 'disabled';
        }
      });
  }

  onCancel(_buttonConfig?: FormPageButtonConfig, _values?: Record<string, any>): void {
    this.router.navigate(['/']);
  }
}
