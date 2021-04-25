/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
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
