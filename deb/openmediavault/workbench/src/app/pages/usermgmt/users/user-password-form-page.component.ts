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
import { Component } from '@angular/core';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';
import { BaseFormPageComponent } from '~/app/pages/base-page-component';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class UserPasswordFormPageComponent extends BaseFormPageComponent {
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
              errorData: gettext("Password doesn't match.")
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
