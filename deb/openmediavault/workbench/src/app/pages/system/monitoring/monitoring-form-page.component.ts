import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class MonitoringFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'PerfStats',
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
        name: 'enable',
        label: gettext('Enabled'),
        value: true,
        hint: gettext(
          'Specifies whether the system performance statistics are collected periodically.'
        )
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
          url: '/system'
        }
      }
    ]
  };
}
