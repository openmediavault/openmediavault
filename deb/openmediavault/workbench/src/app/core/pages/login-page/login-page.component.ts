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
import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { finalize } from 'rxjs/operators';

import {
  FormPageButtonConfig,
  FormPageConfig
} from '~/app/core/components/intuition/models/form-page-config.type';
import { translate } from '~/app/i18n.helper';
import { Icon } from '~/app/shared/enum/icon.enum';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { AuthenticateResponse, AuthService } from '~/app/shared/services/auth.service';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { LocaleService } from '~/app/shared/services/locale.service';
import { NotificationService } from '~/app/shared/services/notification.service';

@Component({
  selector: 'omv-login-page',
  templateUrl: './login-page.component.html',
  styleUrls: ['./login-page.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class LoginPageComponent implements OnInit {
  public currentLocale: string;
  public locales: Record<string, string> = {};
  public icon = Icon;

  public config: FormPageConfig = {
    id: 'login',
    fields: [
      {
        type: 'textInput',
        name: 'username',
        label: gettext('User name'),
        autofocus: true,
        autocomplete: 'username',
        icon: Icon.user,
        validators: {
          required: true
        }
      },
      {
        type: 'passwordInput',
        name: 'password',
        label: gettext('Password'),
        icon: Icon.password,
        autocomplete: 'current-password',
        validators: {
          required: true
        }
      }
    ],
    buttonAlign: 'center',
    buttons: [
      {
        template: 'submit',
        text: gettext('Log in'),
        execute: {
          type: 'click',
          click: this.onLogin.bind(this)
        }
      }
    ]
  };

  constructor(
    private activatedRoute: ActivatedRoute,
    private authService: AuthService,
    private blockUiService: BlockUiService,
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private router: Router
  ) {
    this.currentLocale = LocaleService.getCurrentLocale();
    this.locales = LocaleService.getSupportedLocales();
  }

  ngOnInit(): void {
    this.blockUiService.resetGlobal();
    // Ensure all currently opened dialogs are closed.
    this.dialogService.closeAll();
  }

  onLogin(buttonConfig: FormPageButtonConfig, values: Record<string, any>) {
    this.blockUiService.start(translate(gettext('Please wait ...')));
    this.authService
      .authenticate(values.username, values.password)
      .pipe(
        finalize(() => {
          this.blockUiService.stop();
        })
      )
      .subscribe((res: AuthenticateResponse) => {
        if (res.status === 'authenticated') {
          const url = _.get(this.activatedRoute.snapshot.queryParams, 'returnUrl', '/dashboard');
          this.router.navigate([url]);
        } else if (res.status === 'challengeRequired') {
          this.handleChallenge(res);
        }
      });
  }

  onSelectLocale(locale: string) {
    // Update the browser cookie and reload the page.
    LocaleService.setCurrentLocale(locale);
    this.router.navigate(['/reload']);
  }

  private handleChallenge(authResponse: AuthenticateResponse) {
    if (!authResponse.challenge) {
      this.notificationService.show(
        NotificationType.error,
        gettext('Authentication'),
        gettext(
          'An authentication challenge was required but no challenge information was provided. Please contact your administrator.'
        )
      );
      return;
    }

    if (!authResponse.challenge.redirecturl) {
      this.notificationService.show(
        NotificationType.error,
        gettext('Authentication'),
        gettext(
          'The requested authentication challenge is not supported by this browser interface. Please contact your administrator.'
        )
      );
      return;
    }

    const challengeUrl: string = authResponse.challenge.redirecturl;

    // Redirect MUST be root-relative to prevent open-redirect attacks and to
    // ensure the Angular router can navigate to it.
    if (!challengeUrl.startsWith('/') || challengeUrl.startsWith('//')) {
      this.notificationService.show(
        NotificationType.error,
        gettext('Authentication'),
        gettext('The authentication challenge URL is invalid.')
      );
      return;
    }

    // Use the URL API to parse and validate the URL is same-origin.
    try {
      // Parse as URL with current origin as base. This handles relative URLs.
      const url = new URL(challengeUrl, window.location.origin);

      // Ensure the parsed URL has the same origin (protocol, host, port).
      // This prevents all open redirect attempts:
      // - Absolute URLs to other domains: http://evil.com
      // - Protocol-relative URLs: //evil.com
      // - Path escapes: /\evil.com (browsers normalize this)
      if (url.origin !== window.location.origin) {
        throw new Error('Cross-origin redirect not allowed');
      }
    } catch (e) {
      this.notificationService.show(
        NotificationType.error,
        gettext('Authentication'),
        gettext('The authentication challenge URL is invalid.')
      );
      return;
    }

    this.router.navigateByUrl(challengeUrl);
  }
}
