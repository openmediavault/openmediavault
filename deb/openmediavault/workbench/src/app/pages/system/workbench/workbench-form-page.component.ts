import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class WorkbenchFormPageComponent {
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
        label: gettext('Auto logout'),
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
        type: 'paragraph',
        text: gettext('Secure connection')
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
          max: 65535
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
