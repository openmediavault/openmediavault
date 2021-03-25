import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class FtpShareFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'FTP',
      get: {
        method: 'getShare',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setShare'
      }
    },
    fields: [
      /* eslint-disable max-len */
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
        type: 'sharedFolderSelect',
        name: 'sharedfolderref',
        label: gettext('Shared folder'),
        hint: gettext('The location of the files to share.'),
        validators: {
          requiredIf: { operator: 'eq', arg0: { prop: 'enable' }, arg1: true }
        }
      },
      {
        type: 'textInput',
        name: 'comment',
        label: gettext('Comment'),
        value: ''
      },
      {
        type: 'textarea',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          'Please check the <a href=\'http://www.proftpd.org/docs/directives/linked/by-name.html\' target=\'_blank\'>manual page</a> for more details.'
        ),
        value: ''
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/services/ftp/shares'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/services/ftp/shares'
        }
      }
    ]
  };
}
