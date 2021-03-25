import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class NfsSettingsFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'NFS',
      get: {
        method: 'getSettings'
      },
      post: {
        method: 'setSettings'
      }
    },
    fields: [
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: false
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
          url: '/services/nfs'
        }
      }
    ]
  };
}
