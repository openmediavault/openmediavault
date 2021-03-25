import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class GroupImportFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'UserMgmt',
      post: {
        method: 'importGroups',
        task: true
      }
    },
    fields: [
      {
        type: 'textarea',
        name: 'csv',
        value: '# <name>;<gid>;<comment>',
        hint: gettext('Each line represents a group.'),
        monospaced: true
      }
    ],
    buttons: [
      {
        template: 'submit',
        text: gettext('Import'),
        execute: {
          type: 'url',
          url: '/usermgmt/groups'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/usermgmt/groups'
        }
      }
    ]
  };
}
