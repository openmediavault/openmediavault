import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class SmartTaskFormPageComponent {
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
        type: 'select',
        name: 'dayofmonth',
        label: gettext('Day of month'),
        value: '*',
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
        type: 'select',
        name: 'month',
        label: gettext('Month'),
        value: '*',
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
