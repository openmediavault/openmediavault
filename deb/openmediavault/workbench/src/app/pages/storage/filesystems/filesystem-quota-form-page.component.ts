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
export class FilesystemQuotaFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Quota',
      get: {
        method: 'getByTypeName',
        params: {
          uuid: '{{ _routeParams.uuid }}',
          type: '{{ _routeParams.type }}',
          name: '{{ _routeParams.name }}'
        }
      },
      post: {
        method: 'setByTypeName',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      }
    },
    fields: [
      {
        type: 'select',
        name: 'type',
        label: gettext('Type'),
        disabled: true,
        store: {
          data: [
            ['user', gettext('User')],
            ['group', gettext('Group')]
          ]
        }
      },
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        disabled: true
      },

      {
        type: 'container',
        fields: [
          {
            type: 'numberInput',
            name: 'bhardlimit',
            label: gettext('Quota'),
            value: 0,
            validators: {
              min: 0,
              patternType: 'integer',
              required: true
            },
            flex: 75
          },
          {
            type: 'select',
            name: 'bunit',
            label: gettext('Unit'),
            value: 'GiB',
            store: {
              data: [
                ['KiB', 'KiB'],
                ['MiB', 'MiB'],
                ['GiB', 'GiB'],
                ['TiB', 'TiB']
              ]
            },
            validators: {
              required: true
            },
            flex: 25
          }
        ]
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/filesystems/quota/{{ _routeParams.uuid }}'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/filesystems/quota/{{ _routeParams.uuid }}'
        }
      }
    ]
  };
}
