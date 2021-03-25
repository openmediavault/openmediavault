import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class GeneralNetworkFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Network',
      get: {
        method: 'getGeneralSettings'
      },
      post: {
        method: 'setGeneralSettings'
      }
    },
    fields: [
      {
        type: 'textInput',
        name: 'hostname',
        label: gettext('Hostname'),
        hint: gettext('The hostname is a label that identifies the system to the network.'),
        value: '',
        validators: {
          patternType: 'hostName',
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'domainname',
        label: gettext('Domain name'),
        value: '',
        validators: {
          patternType: 'domainName'
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
          url: '/network'
        }
      }
    ]
  };
}
