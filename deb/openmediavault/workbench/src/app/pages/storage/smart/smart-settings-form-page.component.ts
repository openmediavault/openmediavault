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
export class SmartSettingsFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Smart',
      get: {
        method: 'getSettings'
      },
      post: {
        method: 'setSettings'
      }
    },
    fields: [
      /* eslint-disable max-len */
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: false
      },
      {
        type: 'numberInput',
        name: 'interval',
        label: gettext('Check interval'),
        hint:
          gettext('Sets the interval between disk checks to N seconds.') +
          '&nbsp;' +
          gettext(
            'Take caution when setting this polling interval to more than sixty minutes. The poll times may fail to coincide with any of the scheduled test that have been specified. In this case the scheduled test will be run following the next device polling.'
          ),
        value: 1800,
        validators: {
          min: 10,
          patternType: 'integer',
          required: true
        }
      },
      {
        type: 'select',
        name: 'powermode',
        label: gettext('Power mode'),
        hint: gettext(
          'Prevent a disk from being spun-up when it is periodically polled.<ul><li>Never - Poll (check) the device regardless of its power mode. This may cause a disk which is spun-down to be spun-up when it is checked.</li><li>Sleep - Check the device unless it is in SLEEP mode.</li><li>Standby - Check the device unless it is in SLEEP or STANDBY mode. In these modes most disks are not spinning, so if you want to prevent a disk from spinning up each poll, this is probably what you want.</li><li>Idle - Check the device unless it is in SLEEP, STANDBY or IDLE mode. In the IDLE state, most disks are still spinning, so this is probably not what you want.</li></ul>'
        ),
        value: 'standby',
        store: {
          data: [
            ['never', gettext('Never')],
            ['sleep', gettext('Sleep')],
            ['standby', gettext('Standby')],
            ['idle', gettext('Idle')]
          ]
        }
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
            [0, gettext('Disabled')],
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
            [0, gettext('Disabled')],
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
        template: 'submit'
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/smart'
        }
      }
    ]
  };
}
