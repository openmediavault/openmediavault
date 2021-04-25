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
export class ProxyFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Network',
      get: {
        method: 'getProxy'
      },
      post: {
        method: 'setProxy'
      }
    },
    fields: [
      {
        type: 'paragraph',
        text: gettext('HTTP-Proxy')
      },
      {
        type: 'checkbox',
        name: 'httpenable',
        label: gettext('Enabled'),
        value: false
      },
      {
        type: 'textInput',
        name: 'httphost',
        label: gettext('Host'),
        value: '',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'httpenable' } }
          }
        ],
        validators: {
          patternType: 'domainNameIp',
          requiredIf: { operator: 'truthy', arg0: { prop: 'httpenable' } }
        }
      },
      {
        type: 'numberInput',
        name: 'httpport',
        label: gettext('Port'),
        value: 8080,
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'httpenable' } }
          }
        ],
        validators: {
          patternType: 'port',
          requiredIf: { operator: 'truthy', arg0: { prop: 'httpenable' } },
          min: 1,
          max: 65535
        }
      },
      {
        type: 'textInput',
        name: 'httpusername',
        label: gettext('Username'),
        value: '',
        autocomplete: 'off',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'httpenable' } }
          }
        ],
        validators: {
          patternType: 'userName'
        }
      },
      {
        type: 'passwordInput',
        name: 'httppassword',
        label: gettext('Password'),
        value: '',
        autocomplete: 'new-password',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'httpenable' } }
          }
        ]
      },
      {
        type: 'paragraph',
        text: gettext('HTTPS-Proxy')
      },
      {
        type: 'checkbox',
        name: 'httpsenable',
        label: gettext('Enabled'),
        value: false
      },
      {
        type: 'textInput',
        name: 'httpshost',
        label: gettext('Host'),
        value: '',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'httpsenable' } }
          }
        ],
        validators: {
          patternType: 'domainNameIp',
          requiredIf: { operator: 'truthy', arg0: { prop: 'httpsenable' } }
        }
      },
      {
        type: 'numberInput',
        name: 'httpsport',
        label: gettext('Port'),
        value: 8080,
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'httpsenable' } }
          }
        ],
        validators: {
          patternType: 'port',
          requiredIf: { operator: 'truthy', arg0: { prop: 'httpsenable' } },
          min: 1,
          max: 65535
        }
      },
      {
        type: 'textInput',
        name: 'httpsusername',
        label: gettext('Username'),
        value: '',
        autocomplete: 'off',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'httpsenable' } }
          }
        ],
        validators: {
          patternType: 'userName'
        }
      },
      {
        type: 'passwordInput',
        name: 'httpspassword',
        label: gettext('Password'),
        value: '',
        autocomplete: 'new-password',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'httpsenable' } }
          }
        ]
      },
      {
        type: 'paragraph',
        text: gettext('FTP-Proxy')
      },
      {
        type: 'checkbox',
        name: 'ftpenable',
        label: gettext('Enabled'),
        value: false
      },
      {
        type: 'textInput',
        name: 'ftphost',
        label: gettext('Host'),
        value: '',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'ftpenable' } }
          }
        ],
        validators: {
          patternType: 'domainNameIp',
          requiredIf: { operator: 'truthy', arg0: { prop: 'ftpenable' } }
        }
      },
      {
        type: 'numberInput',
        name: 'ftpport',
        label: gettext('Port'),
        value: 8080,
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'ftpenable' } }
          }
        ],
        validators: {
          patternType: 'port',
          requiredIf: { operator: 'truthy', arg0: { prop: 'ftpenable' } },
          min: 1,
          max: 65535
        }
      },
      {
        type: 'textInput',
        name: 'ftpusername',
        label: gettext('Username'),
        value: '',
        autocomplete: 'off',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'ftpenable' } }
          }
        ],
        validators: {
          patternType: 'userName'
        }
      },
      {
        type: 'passwordInput',
        name: 'ftppassword',
        label: gettext('Password'),
        value: '',
        autocomplete: 'new-password',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'ftpenable' } }
          }
        ]
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
          url: '/network'
        }
      }
    ]
  };
}
