import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class UserPasswordFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'UserMgmt',
      post: {
        method: 'setPasswordByContext'
      }
    },
    fields: [
      {
        type: 'passwordInput',
        name: 'password',
        label: gettext('New password'),
        autocomplete: 'new-password',
        validators: {
          required: true
        }
      },
      {
        type: 'passwordInput',
        name: 'passwordconf',
        label: gettext('Confirm password'),
        submitValue: false,
        validators: {
          required: true,
          custom: [
            {
              constraint: {
                operator: 'eq',
                arg0: { prop: 'password' },
                arg1: { prop: 'passwordconf' }
              },
              errorData: gettext('Password doesn\'t match.')
            }
          ]
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/'
        }
      }
    ]
  };
}
