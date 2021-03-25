import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class SshCertificateImportFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'CertificateMgmt',
      post: {
        method: 'setSsh'
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
      {
        type: 'textarea',
        name: 'privatekey',
        value: '',
        monospaced: true,
        label: gettext('Private key'),
        hint: gettext('The private RSA key in X.509 PEM format.'),
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'publickey',
        value: '',
        monospaced: true,
        label: gettext('Public key'),
        hint: gettext('The RSA public key in OpenSSH format.'),
        validators: {
          required: true
        }
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
        text: gettext('Import'),
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
