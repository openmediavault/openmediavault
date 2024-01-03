/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2024 Volker Theile
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
export class MdRecoverFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    subTitle: gettext('Add hot spares / recover RAID device.'),
    request: {
      service: 'RaidMgmt',
      post: {
        method: 'add'
      }
    },
    fields: [
      {
        type: 'select',
        name: 'devicefile',
        label: gettext('Device'),
        valueField: 'devicefile',
        textField: 'description',
        disabled: true,
        value: '{{ _routeParams.devicefile }}',
        store: {
          proxy: {
            service: 'RaidMgmt',
            get: {
              method: 'enumerateDevices'
            }
          }
        }
      },
      {
        type: 'select',
        name: 'devices',
        label: gettext('Devices'),
        placeholder: gettext('Select devices ...'),
        hint: gettext('Select devices to be added to the RAID device.'),
        multiple: true,
        valueField: 'devicefile',
        textField: 'description',
        store: {
          proxy: {
            service: 'RaidMgmt',
            get: {
              method: 'getCandidates'
            }
          },
          filters: [
            { operator: 'ne', arg0: { prop: 'devicefile' }, arg1: '{{ _routeParams.devicefile }}' }
          ],
          sorters: [
            {
              dir: 'asc',
              prop: 'devicefile'
            }
          ]
        },
        validators: {
          required: true
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/md'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/md'
        }
      }
    ]
  };
}
