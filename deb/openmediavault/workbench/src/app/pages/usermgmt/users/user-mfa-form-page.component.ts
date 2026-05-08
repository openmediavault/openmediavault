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
import { FormControl, Validators } from '@angular/forms';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as QRCode from 'qrcode';
import { finalize } from 'rxjs/operators';

import { translate } from '~/app/i18n.helper';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { RpcService } from '~/app/shared/services/rpc.service';

/**
 * Page component for managing TOTP two-factor authentication on the current
 * user's account. Accessible at /usermgmt/mfa.
 *
 * State machine:
 *  loading   → initial RPC call to fetch current MFA status
 *  disabled  → MFA is off; user can start the setup flow
 *  setup     → secret was generated; QR code is displayed; awaiting confirmation
 *  enabled   → MFA is active; user can disable it
 *
 * Security notes:
 *  - The provisional secret from generateTotpSecretByContext is never stored
 *    until the user confirms it with a valid code (enableTotpByContext).
 *  - Disabling MFA requires submitting the current TOTP code, preventing a
 *    stolen browser session from silently removing MFA protection.
 */
@Component({
  selector: 'omv-user-mfa-form-page',
  templateUrl: './user-mfa-form-page.component.html'
})
export class UserMfaFormPageComponent implements OnInit {
  /** Current UI state. */
  public state: 'loading' | 'disabled' | 'setup' | 'enabled' = 'loading';

  /** Base32-encoded provisional secret returned by the backend during setup. */
  public provisionalSecret = '';

  /** Data URL of the QR code image generated from the otpauth:// URI. */
  public qrCodeDataUrl = '';

  /** Reactive form control for the verification code input during setup. */
  public setupCodeControl = new FormControl('', [
    Validators.required,
    Validators.minLength(6),
    Validators.maxLength(6),
    Validators.pattern(/^\d{6}$/)
  ]);

  /** Reactive form control for the code input when disabling MFA. */
  public disableCodeControl = new FormControl('', [
    Validators.required,
    Validators.minLength(6),
    Validators.maxLength(6),
    Validators.pattern(/^\d{6}$/)
  ]);

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
        // Fall back to disabled view so the page is never stuck loading.
        this.state = 'disabled';
      }
    });
  }

  /**
   * Start the MFA setup flow: ask the backend to generate a fresh secret
   * and render it as a QR code the user can scan with their authenticator app.
   */
  onStartSetup(): void {
    this.blockUiService.start(translate(gettext('Generating secret...')));
    this.rpcService
      .request('UserMgmt', 'generateTotpSecretByContext')
      .pipe(finalize(() => this.blockUiService.stop()))
      .subscribe({
        next: (res: { secret: string; uri: string }) => {
          this.provisionalSecret = res.secret;
          // Generate the QR code image entirely in the browser so the
          // secret never leaves the current page or reaches a third-party.
          QRCode.toDataURL(res.uri, { errorCorrectionLevel: 'M', width: 256 })
            .then((dataUrl: string) => {
              this.qrCodeDataUrl = dataUrl;
              this.setupCodeControl.reset();
              this.state = 'setup';
            })
            .catch(() => {
              this.provisionalSecret = '';
              this.notificationService.show(
                NotificationType.error,
                translate(gettext('Failed to generate QR code. Please try again.'))
              );
            });
        }
      });
  }

  /**
   * Confirm setup: send the provisional secret along with the code the user
   * scanned from the QR image. The backend verifies the code before storing.
   */
  onConfirmSetup(): void {
    if (this.setupCodeControl.invalid) {
      this.setupCodeControl.markAsTouched();
      return;
    }
    this.blockUiService.start(translate(gettext('Enabling two-factor authentication...')));
    this.rpcService
      .request('UserMgmt', 'enableTotpByContext', {
        code:   this.setupCodeControl.value?.trim(),
        secret: this.provisionalSecret
      })
      .pipe(finalize(() => this.blockUiService.stop()))
      .subscribe({
        next: () => {
          this.notificationService.show(
            NotificationType.success,
            translate(gettext('Two-factor authentication has been enabled.'))
          );
          this.state = 'enabled';
          this.qrCodeDataUrl  = '';
          this.provisionalSecret = '';
        }
      });
  }

  /** Cancel setup and return to the disabled view without saving anything. */
  onCancelSetup(): void {
    this.qrCodeDataUrl     = '';
    this.provisionalSecret = '';
    this.setupCodeControl.reset();
    this.state = 'disabled';
  }

  /**
   * Disable TOTP MFA after the user confirms with their current TOTP code.
   * Requiring the code prevents a stolen session from silently removing MFA.
   */
  onDisableMfa(): void {
    if (this.disableCodeControl.invalid) {
      this.disableCodeControl.markAsTouched();
      return;
    }
    this.blockUiService.start(translate(gettext('Disabling two-factor authentication...')));
    this.rpcService
      .request('UserMgmt', 'disableTotpByContext', {
        code: this.disableCodeControl.value?.trim()
      })
      .pipe(finalize(() => this.blockUiService.stop()))
      .subscribe({
        next: () => {
          this.notificationService.show(
            NotificationType.success,
            translate(gettext('Two-factor authentication has been disabled.'))
          );
          this.disableCodeControl.reset();
          this.state = 'disabled';
        }
      });
  }

  onCancel(): void {
    this.router.navigate(['/']);
  }
}
