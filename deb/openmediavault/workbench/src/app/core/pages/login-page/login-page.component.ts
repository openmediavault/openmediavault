import { Component, OnInit } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { finalize } from 'rxjs/operators';

import {
  FormPageButtonConfig,
  FormPageConfig
} from '~/app/core/components/limn-ui/models/form-page-config.type';
import { translate } from '~/app/i18n.helper';
import { Icon } from '~/app/shared/enum/icon.enum';
import { AuthService } from '~/app/shared/services/auth.service';
import { LocaleService } from '~/app/shared/services/locale.service';

@Component({
  selector: 'omv-login-page',
  templateUrl: './login-page.component.html',
  styleUrls: ['./login-page.component.scss']
})
export class LoginPageComponent implements OnInit {
  @BlockUI()
  blockUI: NgBlockUI;

  public currentLocale: string;
  public locales: Record<string, string> = {};

  public config: FormPageConfig = {
    id: 'login',
    fields: [
      {
        type: 'textInput',
        name: 'username',
        label: gettext('Username'),
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
    private router: Router
  ) {
    this.currentLocale = LocaleService.getLocale();
    this.locales = LocaleService.getLocales();
  }

  ngOnInit(): void {
    this.blockUI.resetGlobal();
  }

  onLogin(buttonConfig: FormPageButtonConfig, values: Record<string, any>) {
    this.blockUI.start(translate(gettext('Please wait ...')));
    this.authService
      .login(values.username, values.password)
      .pipe(
        finalize(() => {
          this.blockUI.stop();
        })
      )
      .subscribe(() => {
        const url = _.get(this.activatedRoute.snapshot.queryParams, 'returnUrl', '/');
        this.router.navigate([url]);
      });
  }

  onSelectLocale(locale) {
    // Update the browser cookie and reload the page.
    LocaleService.setLocale(locale);
    document.location.replace('');
  }
}
