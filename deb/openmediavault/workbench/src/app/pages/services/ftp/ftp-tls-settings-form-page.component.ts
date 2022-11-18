/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';
import { IsDirtyFormPageComponent } from '~/app/pages/is-dirty-page-component';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class FtpTlsSettingsFormPageComponent extends IsDirtyFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'FTP',
      get: {
        method: 'getModTLSSettings'
      },
      post: {
        method: 'setModTLSSettings'
      }
    },
    fields: [
      /* eslint-disable max-len */
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        hint: gettext('Enable SSL/TLS connections.'),
        value: false
      },
      {
        type: 'sslCertSelect',
        name: 'sslcertificateref',
        label: gettext('Certificate'),
        hint: gettext('The SSL certificate.'),
        hasEmptyOption: true,
        value: '',
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'falsy', arg0: { prop: 'enable' } }
          }
        ],
        validators: {
          requiredIf: { operator: 'truthy', arg0: { prop: 'enable' } }
        }
      },
      {
        type: 'divider',
        title: gettext('Advanced settings')
      },
      {
        type: 'checkbox',
        name: 'required',
        label: gettext('Required'),
        hint: gettext(
          'This option requires clients to use FTP over TLS when talking to this server.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'nosessionreuserequired',
        label: gettext('No session reuse required'),
        hint: gettext(
          'The requirement that the SSL session from the control connection is reused for data connections is not required.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'useimplicitssl',
        label: gettext('Implicit SSL'),
        hint: gettext(
          'This option will handle all connections as if they are SSL connections implicitly.'
        ),
        value: false
      },
      {
        type: 'textarea',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          "Please check the <a href='http://www.proftpd.org/docs/contrib/mod_tls.html' target='_blank'>manual page</a> for more details."
        ),
        value: ''
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
          url: '/services/ftp'
        }
      }
    ]
  };
}
