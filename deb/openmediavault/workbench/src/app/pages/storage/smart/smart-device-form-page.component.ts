import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class SmartDeviceFormPageComponent {
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
        label: gettext('Monitored'),
        hint: gettext('Activate S.M.A.R.T. monitoring.'),
        value: false
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
