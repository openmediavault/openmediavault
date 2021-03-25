import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class UpdateSettingsFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Apt',
      get: {
        method: 'getSettings'
      },
      post: {
        method: 'setSettings'
      }
    },
    fields: [
      {
        type: 'paragraph',
        text: gettext('Install updates from')
      },
      {
        type: 'checkbox',
        name: 'proposed',
        label: gettext('Pre-release updates'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'partner',
        label: gettext('Community-maintained updates'),
        value: false
      }
    ],
    buttons: [
      {
        template: 'submit',
        confirmationDialogConfig: {
          template: 'confirmation',
          message: gettext(
            'The information about available software is out-of-date. Do you want to reload this information?'
          )
        },
        execute: {
          type: 'request',
          request: {
            service: 'Apt',
            method: 'update',
            task: true,
            progressMessage: gettext(
              'The repository will be checked for new, removed or upgraded software packages.'
            ),
            successUrl: '/system/updatemgmt/updates'
          }
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/updatemgmt'
        }
      }
    ]
  };
}
