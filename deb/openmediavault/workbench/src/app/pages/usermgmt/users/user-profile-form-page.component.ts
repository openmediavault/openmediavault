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
export class UserProfileFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'UserMgmt',
      get: {
        method: 'getUserByContext'
      },
      post: {
        method: 'setUserByContext'
      }
    },
    fields: [
      {
        type: 'hidden',
        name: '_readonly',
        value: false
      },
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        readonly: true
      },
      {
        type: 'textInput',
        name: 'email',
        label: gettext('Email'),
        value: '',
        validators: {
          patternType: 'email'
        }
      },
      {
        type: 'tagInput',
        name: 'comment',
        label: gettext('Tags'),
        value: '',
        validators: {
          maxLength: 65
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        enabledConstraint: { operator: 'falsy', arg0: { prop: '_readonly' } }
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
