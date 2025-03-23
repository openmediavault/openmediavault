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
export class DateTimeFormPageComponent extends BaseFormPageComponent {
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
        hint: gettext('NTP time servers, separated by comma.'),
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
