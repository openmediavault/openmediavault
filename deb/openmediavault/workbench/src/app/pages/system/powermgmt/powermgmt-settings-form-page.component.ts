import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class PowermgmtSettingsFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'PowerMgmt',
      get: {
        method: 'get'
      },
      post: {
        method: 'set'
      }
    },
    fields: [
      {
        type: 'checkbox',
        name: 'cpufreq',
        label: gettext('Monitoring'),
        hint: gettext(
          'Specifies whether to monitor the system status and select the most appropriate CPU level.'
        ),
        value: false
      },
      {
        type: 'select',
        name: 'powerbtn',
        label: gettext('Power button'),
        hint: gettext('The action to be done when pressing the power button.'),
        value: 'nothing',
        store: {
          data: [
            ['nothing', gettext('Nothing')],
            ['shutdown', gettext('Shutdown')],
            ['standby', gettext('Standby')]
          ]
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
          url: '/system/powermgmt'
        }
      }
    ]
  };
}
