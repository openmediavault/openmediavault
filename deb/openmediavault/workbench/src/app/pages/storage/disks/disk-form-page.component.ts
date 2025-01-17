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
export class DiskFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'DiskMgmt',
      get: {
        method: 'getHdParm',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setHdParm'
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
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
            service: 'DiskMgmt',
            get: {
              method: 'enumerateDevices'
            }
          }
        }
      },
      {
        type: 'select',
        name: 'apm',
        label: gettext('Advanced Power Management'),
        hint: gettext('Please note that values >= 128 do not permit the disk to spin down.'),
        value: 0,
        store: {
          data: [
            {
              text: gettext('Disabled'),
              value: 0
            },
            {
              text: gettext('1 - Minimum power usage with standby (spindown)'),
              value: 1
            },
            {
              text: gettext('64 - Intermediate power usage with standby'),
              value: 64
            },
            {
              text: gettext('127 - Intermediate power usage with standby'),
              value: 127
            },
            {
              text: gettext('128 - Minimum power usage without standby (no spindown)'),
              value: 128
            },
            {
              text: gettext('192 - Intermediate power usage without standby'),
              value: 192
            },
            {
              text: gettext('254 - Maximum performance, maximum power usage'),
              value: 254
            },
            {
              text: gettext('255 - Disabled'),
              value: 255
            }
          ]
        }
      },
      {
        type: 'select',
        name: 'aam',
        label: gettext('Advanced Acoustic Management'),
        value: 0,
        store: {
          data: [
            {
              text: gettext('Disabled'),
              value: 0
            },
            {
              text: gettext('Minimum performance, minimum acoustic output'),
              value: 128
            },
            {
              text: gettext('Maximum performance, maximum acoustic output'),
              value: 254
            }
          ]
        }
      },
      {
        type: 'select',
        name: 'spindowntime',
        label: gettext('Spindown time'),
        value: 0,
        store: {
          data: [
            {
              text: gettext('Disabled'),
              value: 0
            },
            {
              text: gettext('5 minutes'),
              value: 60
            },
            {
              text: gettext('10 minutes'),
              value: 120
            },
            {
              text: gettext('20 minutes'),
              value: 240
            },
            {
              text: gettext('30 minutes'),
              value: 241
            },
            {
              text: gettext('60 minutes'),
              value: 242
            },
            {
              text: gettext('120 minutes'),
              value: 244
            },
            {
              text: gettext('180 minutes'),
              value: 246
            },
            {
              text: gettext('240 minutes'),
              value: 248
            },
            {
              text: gettext('300 minutes'),
              value: 250
            },
            {
              text: gettext('330 minutes'),
              value: 251
            }
          ]
        }
      },
      {
        type: 'checkbox',
        name: 'writecache',
        label: gettext('Enable write-cache'),
        hint: gettext('This function is effective only if the hard drive supports it.'),
        value: false
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/disks'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/disks'
        }
      }
    ]
  };
}
