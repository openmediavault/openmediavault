import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class SshCertificateEditFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'CertificateMgmt',
      get: {
        method: 'getSsh',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setSsh'
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
      {
        type: 'textInput',
        name: 'publickey',
        value: '',
        monospaced: true,
        hasCopyToClipboardButton: true,
        label: gettext('Public key'),
        hint: gettext('The RSA public key in OpenSSH format.'),
        disabled: true
      },
      {
        type: 'textInput',
        name: 'comment',
        value: '',
        label: gettext('Comment'),
        validators: {
          required: true
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/system/certificate/ssh'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/certificate/ssh'
        }
      }
    ]
  };
}
