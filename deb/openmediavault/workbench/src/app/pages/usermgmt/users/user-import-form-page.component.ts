import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class UserImportFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'UserMgmt',
      post: {
        method: 'importUsers',
        task: true
      }
    },
    fields: [
      {
        type: 'textarea',
        name: 'csv',
        value:
          '# <name>;<uid>;<comment>;<email>;<password>;<shell>;<group,group,...>;<disallowusermod>',
        hint: gettext(
          'Each line represents a user. Note, the password must be entered in plain text.'
        ),
        monospaced: true
      }
    ],
    buttons: [
      {
        template: 'submit',
        text: gettext('Import'),
        execute: {
          type: 'url',
          url: '/usermgmt/users'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/usermgmt/users'
        }
      }
    ]
  };
}
