/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
import { AuthService } from '~/app/shared/services/auth.service';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { LocaleService } from '~/app/shared/services/locale.service';

@Component({
  selector: 'omv-login-page',
  templateUrl: './login-page.component.html',
  styleUrls: ['./login-page.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class LoginPageComponent implements OnInit {
  public currentLocale: string;
  public locales: Record<string, string> = {};

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
      .login(values.username, values.password)
      .pipe(
        finalize(() => {
          this.blockUiService.stop();
        })
      )
      .subscribe(() => {
        const url = _.get(this.activatedRoute.snapshot.queryParams, 'returnUrl', '/dashboard');
        this.router.navigate([url]);
      });
  }

  onSelectLocale(locale) {
    // Update the browser cookie and reload the page.
    LocaleService.setCurrentLocale(locale);
    this.router.navigate(['/reload']);
  }
}
