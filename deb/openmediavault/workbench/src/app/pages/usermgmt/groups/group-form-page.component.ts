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
export class GroupFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'UserMgmt',
      get: {
        method: 'getGroup',
        params: {
          name: '{{ _routeParams.name }}'
        }
      },
      post: {
        method: 'setGroup'
      }
    },
    fields: [
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        disabled: '{{ _routeConfig.data.editing | toboolean }}',
        value: '{{ _routeParams.name }}',
        validators: {
          required: true,
          patternType: 'groupName'
        }
      },
      {
        type: 'select',
        name: 'members',
        label: gettext('Members'),
        placeholder: gettext('Select members ...'),
        multiple: true,
        valueField: 'name',
        textField: 'name',
        value: [],
        store: {
          proxy: {
            service: 'UserMgmt',
            get: {
              method: 'enumerateAllUsers'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'name'
            }
          ]
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
        execute: {
          type: 'url',
          url: '/usermgmt/groups'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/usermgmt/groups'
        }
      }
    ]
  };
}
