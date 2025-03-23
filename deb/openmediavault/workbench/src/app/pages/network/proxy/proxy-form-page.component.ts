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
export class ProxyFormPageComponent extends BaseFormPageComponent {
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
        title: gettext('HTTP-Proxy')
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
        label: gettext('User name'),
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
        type: 'divider',
        title: gettext('HTTPS-Proxy')
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
        label: gettext('User name'),
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
        type: 'divider',
        title: gettext('FTP-Proxy')
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
        label: gettext('User name'),
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
