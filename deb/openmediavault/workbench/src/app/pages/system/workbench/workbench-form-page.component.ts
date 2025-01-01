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
export class WorkbenchFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'WebGui',
      get: {
        method: 'getSettings'
      },
      post: {
        method: 'setSettings'
      }
    },
    fields: [
      {
        type: 'numberInput',
        name: 'port',
        label: gettext('Port'),
        value: 80,
        validators: {
          min: 0,
          max: 65535,
          required: true,
          patternType: 'port'
        }
      },
      {
        type: 'select',
        name: 'timeout',
        label: gettext('Automatic logout'),
        hint: gettext('Close the session on inactivity after the specified time.'),
        value: 5,
        store: {
          data: [
            {
              text: gettext('Disabled'),
              value: 0
            },
            {
              text: gettext('1 minute'),
              value: 1
            },
            {
              text: gettext('2 minutes'),
              value: 2
            },
            {
              text: gettext('3 minutes'),
              value: 3
            },
            {
              text: gettext('4 minutes'),
              value: 4
            },
            {
              text: gettext('5 minutes'),
              value: 5
            },
            {
              text: gettext('10 minutes'),
              value: 10
            },
            {
              text: gettext('15 minutes'),
              value: 15
            },
            {
              text: gettext('30 minutes'),
              value: 30
            },
            {
              text: gettext('60 minutes'),
              value: 60
            },
            {
              text: gettext('1 day'),
              value: 1440
            }
          ]
        }
      },
      {
        type: 'divider',
        title: gettext('Secure connection')
      },
      {
        type: 'checkbox',
        name: 'enablessl',
        label: gettext('SSL/TLS enabled'),
        value: false
      },
      {
        type: 'sslCertSelect',
        name: 'sslcertificateref',
        label: gettext('Certificate'),
        hasEmptyOption: true,
        value: '',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'enablessl' } }
          }
        ],
        validators: {
          requiredIf: { operator: 'truthy', arg0: { prop: 'enablessl' } }
        }
      },
      {
        type: 'numberInput',
        name: 'sslport',
        label: gettext('Port'),
        value: 443,
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'enablessl' } }
          }
        ],
        validators: {
          requiredIf: { operator: 'truthy', arg0: { prop: 'enablessl' } },
          min: 0,
          max: 65535,
          patternType: 'port'
        }
      },
      {
        type: 'checkbox',
        name: 'forcesslonly',
        label: gettext('Force SSL/TLS'),
        hint: gettext('Force secure connection only.'),
        value: false,
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'enablessl' } }
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
          url: '/system'
        }
      }
    ]
  };
}
