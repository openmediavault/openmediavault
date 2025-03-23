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
export class SmartTaskFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Smart',
      get: {
        method: 'getScheduledTest',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setScheduledTest'
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: true
      },
      {
        type: 'select',
        name: 'devicefile',
        label: gettext('Device'),
        placeholder: gettext('Select a device ...'),
        hint: gettext('S.M.A.R.T. monitoring must be activated for the selected device.'),
        valueField: 'devicefile',
        textField: 'description',
        store: {
          proxy: {
            service: 'Smart',
            get: {
              method: 'enumerateMonitoredDevices'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'description'
            }
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'select',
        name: 'type',
        label: gettext('Type'),
        value: 'S',
        store: {
          data: [
            ['S', gettext('Short self-test')],
            ['L', gettext('Long self-test')],
            ['C', gettext('Conveyance self-test')],
            ['O', gettext('Offline immediate test')]
          ]
        }
      },
      {
        type: 'select',
        name: 'hour',
        label: gettext('Hour'),
        value: '{{ moment("H:mm:ss") | split(":") | get(0) }}',
        store: {
          data: [
            ['*', '*'],
            ['00', '0'],
            ['01', '1'],
            ['02', '2'],
            ['03', '3'],
            ['04', '4'],
            ['05', '5'],
            ['06', '6'],
            ['07', '7'],
            ['08', '8'],
            ['09', '9'],
            ['10', '10'],
            ['11', '11'],
            ['12', '12'],
            ['13', '13'],
            ['14', '14'],
            ['15', '15'],
            ['16', '16'],
            ['17', '17'],
            ['18', '18'],
            ['19', '19'],
            ['20', '20'],
            ['21', '21'],
            ['22', '22'],
            ['23', '23']
          ]
        }
      },
      {
        type: 'select',
        name: 'dayofmonth',
        label: gettext('Day of month'),
        value: '*',
        store: {
          data: [
            ['*', '*'],
            ['01', '1'],
            ['02', '2'],
            ['03', '3'],
            ['04', '4'],
            ['05', '5'],
            ['06', '6'],
            ['07', '7'],
            ['08', '8'],
            ['09', '9'],
            ['10', '10'],
            ['11', '11'],
            ['12', '12'],
            ['13', '13'],
            ['14', '14'],
            ['15', '15'],
            ['16', '16'],
            ['17', '17'],
            ['18', '18'],
            ['19', '19'],
            ['20', '20'],
            ['21', '21'],
            ['22', '22'],
            ['23', '23'],
            ['24', '24'],
            ['25', '25'],
            ['26', '26'],
            ['27', '27'],
            ['28', '28'],
            ['29', '29'],
            ['30', '30'],
            ['31', '31']
          ]
        }
      },
      {
        type: 'select',
        name: 'month',
        label: gettext('Month'),
        value: '*',
        store: {
          data: [
            ['*', '*'],
            ['01', gettext('January')],
            ['02', gettext('February')],
            ['03', gettext('March')],
            ['04', gettext('April')],
            ['05', gettext('May')],
            ['06', gettext('June')],
            ['07', gettext('July')],
            ['08', gettext('August')],
            ['09', gettext('September')],
            ['10', gettext('October')],
            ['11', gettext('November')],
            ['12', gettext('December')]
          ]
        }
      },
      {
        type: 'select',
        name: 'dayofweek',
        label: gettext('Day of week'),
        value: '*',
        store: {
          data: [
            ['*', '*'],
            ['1', gettext('Monday')],
            ['2', gettext('Tuesday')],
            ['3', gettext('Wednesday')],
            ['4', gettext('Thursday')],
            ['5', gettext('Friday')],
            ['6', gettext('Saturday')],
            ['7', gettext('Sunday')]
          ]
        }
      },
      {
        type: 'tagInput',
        name: 'comment',
        label: gettext('Tags'),
        value: ''
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/smart/tasks'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/smart/tasks'
        }
      }
    ]
  };
}
