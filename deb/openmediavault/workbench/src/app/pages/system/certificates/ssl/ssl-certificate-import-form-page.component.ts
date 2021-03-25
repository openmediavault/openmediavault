import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class SslCertificateImportFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'CertificateMgmt',
      post: {
        method: 'set'
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
        hint: gettext('Paste a RSA or ECC private key in X.509 PEM format here.'),
        validators: {
          required: true
        }
      },
      {
        type: 'textarea',
        name: 'certificate',
        value: '',
        monospaced: true,
        label: gettext('Certificate'),
        hint: gettext('Paste a RSA or ECC certificate in X.509 PEM format here.'),
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
          url: '/system/certificate/ssl'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/certificate/ssl'
        }
      }
    ]
  };
}
