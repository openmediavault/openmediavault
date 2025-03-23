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

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class InterfaceDetailsFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Network',
      get: {
        method: 'getInformation',
        params: {
          devicename: '{{ _routeParams.devicename }}'
        },
        transform: {
          prefix: '{{ prefix | replace("-1", "") }}',
          prefix6: '{{ prefix6 | replace("-1", "") }}'
        }
      }
    },
    fields: [
      {
        type: 'select',
        name: 'type',
        label: gettext('Type'),
        disabled: true,
        submitValue: false,
        value: 'bond',
        store: {
          data: [
            ['ethernet', gettext('Ethernet')],
            ['bond', gettext('Bond')],
            ['vlan', gettext('VLAN')],
            ['wifi', gettext('Wi-Fi')],
            ['bridge', gettext('Bridge')]
          ]
        }
      },
      {
        type: 'textInput',
        name: 'devicename',
        label: gettext('Device'),
        disabled: true
      },
      {
        type: 'textInput',
        name: 'ether',
        label: gettext('Hardware Address'),
        disabled: true,
        hasCopyToClipboardButton: true
      },
      {
        type: 'textInput',
        name: 'mtu',
        label: gettext('MTU'),
        disabled: true
      },
      {
        type: 'textInput',
        name: 'state',
        label: gettext('State'),
        disabled: true
      },
      {
        type: 'divider',
        title: gettext('IPv4')
      },
      {
        type: 'textInput',
        name: 'address',
        label: gettext('Address'),
        disabled: true,
        hasCopyToClipboardButton: true
      },
      {
        type: 'textInput',
        name: 'prefix',
        label: gettext('Prefix length'),
        disabled: true
      },
      {
        type: 'textInput',
        name: 'netmask',
        label: gettext('Prefix address'),
        disabled: true
      },
      {
        type: 'textInput',
        name: 'gateway',
        label: gettext('Gateway'),
        disabled: true,
        hasCopyToClipboardButton: true
      },
      {
        type: 'divider',
        title: gettext('IPv6')
      },
      {
        type: 'textInput',
        name: 'address6',
        label: gettext('Address'),
        disabled: true,
        hasCopyToClipboardButton: true
      },
      {
        type: 'textInput',
        name: 'prefix6',
        label: gettext('Prefix length'),
        disabled: true
      },
      {
        type: 'textInput',
        name: 'netmask6',
        label: gettext('Prefix address'),
        disabled: true
      },
      {
        type: 'textInput',
        name: 'gateway6',
        label: gettext('Gateway'),
        disabled: true,
        hasCopyToClipboardButton: true
      }
    ]
  };
}
