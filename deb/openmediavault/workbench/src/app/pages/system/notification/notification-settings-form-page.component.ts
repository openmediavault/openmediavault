/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
import { BaseFormPageComponent } from '~/app/pages/base-page-component';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class NotificationSettingsFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'EmailNotification',
      get: {
        method: 'get'
      },
      post: {
        method: 'set'
      }
    },
    fields: [
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: false
      },
      {
        type: 'textInput',
        name: 'server',
        label: gettext('SMTP server'),
        hint: gettext('Outgoing SMTP mail server address, e.g. smtp.mycorp.com.'),
        value: '',
        validators: {
          requiredIf: { operator: 'eq', arg0: { prop: 'enable' }, arg1: true },
          patternType: 'domainNameIp'
        }
      },
      {
        type: 'numberInput',
        name: 'port',
        label: gettext('SMTP port'),
        hint: gettext('The default SMTP mail server port, e.g. 25, 465 or 587.'),
        value: 25,
        validators: {
          min: 1,
          max: 65535,
          required: true,
          patternType: 'port'
        }
      },
      {
        type: 'select',
        name: 'tls',
        label: gettext('Encryption mode'),
        value: 'none',
        store: {
          data: [
            ['none', gettext('None')],
            ['ssl', gettext('SSL/TLS')],
            ['starttls', gettext('STARTTLS')],
            ['auto', gettext('Automatic')]
          ]
        }
      },
      {
        type: 'textInput',
        name: 'sender',
        label: gettext('Sender email'),
        value: '',
        validators: {
          requiredIf: { operator: 'truthy', arg0: { prop: 'enable' } },
          patternType: 'email'
        }
      },
      {
        type: 'checkbox',
        name: 'authenable',
        label: gettext('Authentication required'),
        value: false
      },
      {
        type: 'textInput',
        name: 'username',
        label: gettext('User name'),
        value: '',
        autocomplete: 'off',
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'falsy', arg0: { prop: 'authenable' } }
          }
        ],
        validators: {
          requiredIf: { operator: 'truthy', arg0: { prop: 'authenable' } }
        }
      },
      {
        type: 'passwordInput',
        name: 'password',
        label: gettext('Password'),
        value: '',
        autocomplete: 'new-password',
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'falsy', arg0: { prop: 'authenable' } }
          }
        ],
        validators: {
          requiredIf: { operator: 'truthy', arg0: { prop: 'authenable' } }
        }
      },
      {
        type: 'divider',
        title: gettext('Recipient')
      },
      {
        type: 'textInput',
        name: 'primaryemail',
        label: gettext('Primary email'),
        value: '',
        validators: {
          requiredIf: { operator: 'truthy', arg0: { prop: 'enable' } },
          patternType: 'email'
        }
      },
      {
        type: 'textInput',
        name: 'secondaryemail',
        label: gettext('Secondary email'),
        value: '',
        validators: {
          patternType: 'email'
        }
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
          url: '/system/notification'
        }
      },
      {
        text: gettext('Test'),
        enabledConstraint: { operator: 'truthy', arg0: { prop: 'enable' } },
        execute: {
          type: 'request',
          request: {
            service: 'EmailNotification',
            method: 'sendTestEmail',
            progressMessage: gettext('Please wait, sending a test email ...'),
            successNotification: gettext(
              // eslint-disable-next-line max-len
              'An attempt to send the test email has been made. Please check your mailbox. If the email does not arrive, check any spam folders and also check there are no Postfix related errors in the system log.'
            )
          }
        }
      }
    ]
  };
}
