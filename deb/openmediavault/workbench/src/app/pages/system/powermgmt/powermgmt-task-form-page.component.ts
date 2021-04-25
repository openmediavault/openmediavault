/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class PowermgmtTaskFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'PowerMgmt',
      get: {
        method: 'getScheduledJob',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setScheduledJob'
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
        name: 'type',
        label: gettext('Type'),
        value: 'reboot',
        store: {
          data: [
            ['standby', gettext('Standby')],
            ['shutdown', gettext('Shutdown')],
            ['reboot', gettext('Reboot')]
          ]
        }
      },
      {
        type: 'select',
        name: 'execution',
        label: gettext('Time of execution'),
        value: 'exactly',
        store: {
          data: [
            ['exactly', gettext('Certain date')],
            ['hourly', gettext('Hourly')],
            ['daily', gettext('Daily')],
            ['weekly', gettext('Weekly')],
            ['monthly', gettext('Monthly')],
            ['yearly', gettext('Yearly')]
          ]
        }
      },
      {
        type: 'container',
        fields: [
          {
            type: 'select',
            name: 'minute',
            label: gettext('Minute'),
            value: '{{ moment("H:mm:ss") | split(":") | get(1) }}',
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ],
            validators: {
              requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' }
            },
            store: {
              data: [
                ['*', '*'],
                ['0', '0'],
                ['1', '1'],
                ['2', '2'],
                ['3', '3'],
                ['4', '4'],
                ['5', '5'],
                ['6', '6'],
                ['7', '7'],
                ['8', '8'],
                ['9', '9'],
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
                ['31', '31'],
                ['32', '32'],
                ['33', '33'],
                ['34', '34'],
                ['35', '35'],
                ['36', '36'],
                ['37', '37'],
                ['38', '38'],
                ['39', '39'],
                ['40', '40'],
                ['41', '41'],
                ['42', '42'],
                ['43', '43'],
                ['44', '44'],
                ['45', '45'],
                ['46', '46'],
                ['47', '47'],
                ['48', '48'],
                ['49', '49'],
                ['50', '50'],
                ['51', '51'],
                ['52', '52'],
                ['53', '53'],
                ['54', '54'],
                ['55', '55'],
                ['56', '56'],
                ['57', '57'],
                ['58', '58'],
                ['59', '59']
              ]
            }
          },
          {
            type: 'checkbox',
            name: 'everynminute',
            label: gettext('Every N minute'),
            value: false,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ]
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'select',
            name: 'hour',
            label: gettext('Hour'),
            value: '{{ moment("H:mm:ss") | split(":") | get(0) }}',
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ],
            validators: {
              requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' }
            },
            store: {
              data: [
                ['*', '*'],
                ['0', '0'],
                ['1', '1'],
                ['2', '2'],
                ['3', '3'],
                ['4', '4'],
                ['5', '5'],
                ['6', '6'],
                ['7', '7'],
                ['8', '8'],
                ['9', '9'],
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
            type: 'checkbox',
            name: 'everynhour',
            label: gettext('Every N hour'),
            value: false,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ]
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'select',
            name: 'dayofmonth',
            label: gettext('Day of month'),
            value: '*',
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ],
            validators: {
              requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' }
            },
            store: {
              data: [
                ['*', '*'],
                ['1', '1'],
                ['2', '2'],
                ['3', '3'],
                ['4', '4'],
                ['5', '5'],
                ['6', '6'],
                ['7', '7'],
                ['8', '8'],
                ['9', '9'],
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
            type: 'checkbox',
            name: 'everyndayofmonth',
            label: gettext('Every N day of month'),
            value: false,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
              }
            ]
          }
        ]
      },
      {
        type: 'select',
        name: 'month',
        label: gettext('Month'),
        value: '*',
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
          }
        ],
        validators: {
          requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' }
        },
        store: {
          data: [
            ['*', '*'],
            ['1', gettext('January')],
            ['2', gettext('February')],
            ['3', gettext('March')],
            ['4', gettext('April')],
            ['5', gettext('May')],
            ['6', gettext('June')],
            ['7', gettext('July')],
            ['8', gettext('August')],
            ['9', gettext('September')],
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
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'execution' }, arg1: 'exactly' }
          }
        ],
        validators: {
          requiredIf: { operator: 'eq', arg0: { prop: 'execution' }, arg1: 'exactly' }
        },
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
        type: 'textInput',
        name: 'comment',
        label: gettext('Comment'),
        value: ''
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/system/powermgmt/tasks'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/powermgmt/tasks'
        }
      }
    ]
  };
}
