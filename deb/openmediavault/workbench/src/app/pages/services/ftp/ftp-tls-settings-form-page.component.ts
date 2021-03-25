import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class FtpTlsSettingsFormPageComponent {
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
        type: 'paragraph',
        text: gettext('Advanced settings')
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
          'Please check the <a href=\'http://www.proftpd.org/docs/contrib/mod_tls.html\' target=\'_blank\'>manual page</a> for more details.'
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
