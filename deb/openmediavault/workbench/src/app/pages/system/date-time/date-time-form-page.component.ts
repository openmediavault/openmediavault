import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class DateTimeFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'System',
      get: {
        method: 'getTimeSettings'
      },
      post: {
        method: 'setTimeSettings'
      }
    },
    fields: [
      {
        type: 'select',
        name: 'timezone',
        label: gettext('Time zone'),
        store: {
          proxy: {
            service: 'System',
            get: {
              method: 'getTimeZoneList'
            }
          }
        },
        textField: 'value',
        value: 'UTC'
      },
      {
        type: 'checkbox',
        name: 'ntpenable',
        label: gettext('Use NTP server'),
        value: false
      },
      {
        type: 'textInput',
        name: 'ntptimeservers',
        label: gettext('Time servers'),
        value: 'pool.ntp.org',
        modifiers: [
          {
            type: 'enabled',
            constraint: { operator: 'truthy', arg0: { prop: 'ntpenable' } }
          }
        ],
        validators: {
          requiredIf: { operator: 'truthy', arg0: { prop: 'ntpenable' } },
          patternType: 'domainNameIpList'
        }
      },
      {
        type: 'textInput',
        name: 'ntpclients',
        label: gettext('Allowed clients'),
        hint: gettext(
          'IP addresses in CIDR notation or host names of clients that are allowed to access the NTP server.'
        ),
        value: '',
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'falsy', arg0: { prop: 'ntpenable' } }
          }
        ],
        validators: {
          patternType: 'hostNameIpNetCidrList'
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
          url: '/system'
        }
      }
    ]
  };
}
