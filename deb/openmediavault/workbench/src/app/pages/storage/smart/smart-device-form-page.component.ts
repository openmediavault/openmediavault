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
export class SmartDeviceFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Smart',
      get: {
        method: 'getDeviceSettings',
        params: {
          devicefile: '{{ _routeParams.devicefile }}'
        }
      },
      post: {
        method: 'setDeviceSettings'
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
      {
        type: 'hidden',
        name: '_used'
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
            service: 'Smart',
            get: {
              method: 'enumerateDevices'
            }
          }
        }
      },
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Monitoring enabled'),
        hint: gettext(
          'Activate S.M.A.R.T. monitoring for this device. Note that only monitored devices are listed in the scheduled tasks. Monitoring cannot be switched off as long as there are scheduled tasks for this device.'
        ),
        value: false,
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'truthy', arg0: { prop: '_used' } }
          }
        ]
      },
      {
        type: 'divider',
        title: gettext('Temperature monitoring')
      },
      {
        type: 'select',
        name: 'tempdiff',
        label: gettext('Difference'),
        hint: gettext(
          'Report if the temperature had changed by at least N degrees Celsius since last report.'
        ),
        value: 0,
        store: {
          data: [
            [0, gettext('Use global settings')],
            [1, '1 °C'],
            [2, '2 °C'],
            [3, '3 °C'],
            [4, '4 °C'],
            [5, '5 °C'],
            [6, '6 °C'],
            [7, '7 °C'],
            [8, '8 °C'],
            [9, '9 °C'],
            [10, '10 °C'],
            [11, '11 °C'],
            [12, '12 °C'],
            [13, '13 °C'],
            [14, '14 °C'],
            [15, '15 °C']
          ]
        },
        validators: {
          min: 0,
          required: true
        }
      },
      {
        type: 'select',
        name: 'tempmax',
        label: gettext('Maximum'),
        hint: gettext('Report if the temperature is greater than or equal to N degrees Celsius.'),
        value: 0,
        store: {
          data: [
            [0, gettext('Use global settings')],
            [5, '5 °C'],
            [10, '10 °C'],
            [15, '15 °C'],
            [20, '20 °C'],
            [25, '25 °C'],
            [30, '30 °C'],
            [35, '35 °C'],
            [40, '40 °C'],
            [45, '45 °C'],
            [50, '50 °C'],
            [55, '55 °C'],
            [60, '60 °C'],
            [65, '65 °C'],
            [70, '70 °C'],
            [75, '75 °C'],
            [80, '80 °C'],
            [85, '85 °C'],
            [90, '90 °C'],
            [95, '95 °C'],
            [100, '100 °C']
          ]
        },
        validators: {
          min: 0,
          required: true
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/smart/devices'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/smart/devices'
        }
      }
    ]
  };
}
