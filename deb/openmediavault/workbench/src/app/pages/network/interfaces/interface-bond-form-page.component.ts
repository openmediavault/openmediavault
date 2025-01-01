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
export class InterfaceBondFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Network',
      get: {
        method: 'getBondIface',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setBondIface',
        params: {
          wol: false
        }
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
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
        value: '',
        disabled: true,
        modifiers: [
          {
            type: 'hidden',
            constraint: { operator: 'falsy', arg0: { prop: '_editing' } }
          }
        ]
      },
      {
        type: 'select',
        name: 'slaves',
        label: gettext('Slaves'),
        placeholder: gettext('Select slave devices ...'),
        hint: gettext('Specifies the slave devices.'),
        multiple: true,
        valueField: 'devicename',
        textField: 'description',
        value: [],
        store: {
          proxy: {
            service: 'Network',
            get: {
              method: 'enumerateBondSlaves',
              params: {
                uuid: '{{ _routeParams.uuid | default(newconfobjuuid) }}',
                unused: true
              }
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'devicename'
            }
          ]
        },
        validators: {
          required: true,
          custom: [
            {
              constraint: {
                operator: '>=',
                arg0: {
                  operator: 'length',
                  arg0: { prop: 'slaves' }
                },
                arg1: 1
              },
              errorData: gettext('At least one network interface is required.')
            }
          ]
        }
      },
      {
        type: 'select',
        name: 'bondmode',
        label: gettext('Mode'),
        value: 1,
        store: {
          data: [
            [0, 'balance-rr'],
            [1, 'active-backup'],
            [2, 'balance-xor'],
            [3, 'broadcast'],
            [4, '802.3ad'],
            [5, 'balance-tlb'],
            [6, 'balance-alb']
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'select',
        name: 'bondtransmithashpolicy',
        label: gettext('Transmit Hash Policy'),
        value: 'layer2',
        store: {
          data: [
            ['layer2', 'layer2'],
            ['layer2+3', 'layer2+3'],
            ['layer3+4', 'layer3+4'],
            ['encap2+3', 'encap2+3'],
            ['encap3+4', 'encap3+4']
          ]
        },
        modifiers: [
          {
            type: 'enabled',
            constraint: {
              operator: 'in',
              arg0: { prop: 'bondmode' },
              arg1: [2, 4, 5]
            }
          }
        ]
      },
      {
        type: 'textInput',
        name: 'bondprimary',
        label: gettext('Primary'),
        hint: gettext('Specifies which slave is the primary device.'),
        value: '',
        modifiers: [
          {
            type: 'value',
            typeConfig: '{{ slaves | get(0) }}',
            constraint: {
              operator: 'and',
              arg0: {
                operator: 'in',
                arg0: { prop: 'bondmode' },
                arg1: [1, 5, 6]
              },
              arg1: {
                operator: 'and',
                arg0: { operator: 'notEmpty', arg0: { prop: 'slaves' } },
                arg1: { operator: 'empty', arg0: { prop: 'bondprimary' } }
              }
            }
          },
          {
            type: 'value',
            typeConfig: '',
            constraint: {
              operator: 'not',
              arg0: {
                operator: 'in',
                arg0: { prop: 'bondmode' },
                arg1: [1, 5, 6]
              }
            }
          },
          {
            type: 'enabled',
            constraint: {
              operator: 'in',
              arg0: { prop: 'bondmode' },
              arg1: [1, 5, 6]
            }
          }
        ],
        validators: {
          requiredIf: {
            operator: 'in',
            arg0: { prop: 'bondmode' },
            arg1: [1, 5, 6]
          },
          custom: [
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'and',
                  arg0: { operator: 'notEmpty', arg0: { prop: 'slaves' } },
                  arg1: {
                    operator: 'in',
                    arg0: { prop: 'bondmode' },
                    arg1: [1, 5, 6]
                  }
                },
                arg1: {
                  operator: 'and',
                  arg0: { operator: 'notEmpty', arg0: { prop: 'bondprimary' } },
                  arg1: {
                    operator: 'in',
                    arg0: { prop: 'bondprimary' },
                    arg1: { prop: 'slaves' }
                  }
                }
              },
              errorData: gettext(
                'The value must be on of the slave network interfaces {{ slaves | join(", ") }}.'
              )
            },
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'and',
                  arg0: { operator: 'empty', arg0: { prop: 'slaves' } },
                  arg1: {
                    operator: 'in',
                    arg0: { prop: 'bondmode' },
                    arg1: [1, 5, 6]
                  }
                },
                arg1: {
                  operator: 'in',
                  arg0: { prop: 'bondprimary' },
                  arg1: { prop: 'slaves' }
                }
              },
              errorData: gettext('The value must be on of the slave network interfaces.')
            }
          ]
        }
      },
      {
        type: 'numberInput',
        name: 'bondmiimon',
        label: gettext('MII monitoring frequency'),
        hint: gettext('Specifies the MII link monitoring frequency in milliseconds.'),
        value: 100,
        validators: {
          min: 0,
          patternType: 'integer'
        }
      },
      {
        type: 'numberInput',
        name: 'bonddowndelay',
        label: gettext('Down delay'),
        hint: gettext(
          'Specifies the time, in milliseconds, to wait before disabling a slave after a link failure has been detected.'
        ),
        value: 200,
        validators: {
          min: 0,
          patternType: 'integer'
        }
      },
      {
        type: 'numberInput',
        name: 'bondupdelay',
        label: gettext('Up delay'),
        hint: gettext(
          'Specifies the time, in milliseconds, to wait before enabling a slave after a link recovery has been detected.'
        ),
        value: 200,
        validators: {
          min: 0,
          patternType: 'integer'
        }
      },
      {
        type: 'tagInput',
        name: 'comment',
        label: gettext('Tags'),
        value: ''
      },
      {
        type: 'divider',
        title: gettext('IPv4')
      },
      {
        type: 'select',
        name: 'method',
        label: gettext('Method'),
        value: 'manual',
        store: {
          data: [
            ['manual', gettext('Disabled')],
            ['dhcp', gettext('DHCP')],
            ['static', gettext('Static')]
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'address',
        label: gettext('Address'),
        value: '',
        validators: {
          patternType: 'ipv4',
          requiredIf: { operator: 'eq', arg0: { prop: 'method' }, arg1: 'static' }
        },
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'method' }, arg1: 'static' }
          }
        ]
      },
      {
        type: 'textInput',
        name: 'netmask',
        label: gettext('Netmask'),
        value: '',
        validators: {
          patternType: 'netmask',
          requiredIf: { operator: 'eq', arg0: { prop: 'method' }, arg1: 'static' }
        },
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'method' }, arg1: 'static' }
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'textInput',
            name: 'gateway',
            label: gettext('Gateway'),
            value: '',
            validators: {
              patternType: 'ipv4'
            },
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'method' }, arg1: 'static' }
              }
            ],
            flex: 75
          },
          {
            type: 'numberInput',
            name: 'routemetric',
            label: gettext('Metric'),
            value: 0,
            validators: {
              min: 0,
              max: 65535,
              patternType: 'integer'
            },
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'method' }, arg1: 'static' }
              }
            ]
          }
        ]
      },
      {
        type: 'divider',
        title: gettext('IPv6')
      },
      {
        type: 'select',
        name: 'method6',
        label: gettext('Method'),
        value: 'manual',
        store: {
          data: [
            ['manual', gettext('Disabled')],
            ['dhcp', gettext('DHCP')],
            ['auto', gettext('Automatic')],
            ['static', gettext('Static')]
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'address6',
        label: gettext('Address'),
        value: '',
        validators: {
          patternType: 'ipv6',
          requiredIf: { operator: 'eq', arg0: { prop: 'method6' }, arg1: 'static' }
        },
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'method6' }, arg1: 'static' }
          }
        ]
      },
      {
        type: 'numberInput',
        name: 'netmask6',
        label: gettext('Prefix length'),
        value: 64,
        validators: {
          min: 0,
          max: 128,
          patternType: 'integer',
          requiredIf: { operator: 'eq', arg0: { prop: 'method6' }, arg1: 'static' }
        },
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'ne', arg0: { prop: 'method6' }, arg1: 'static' }
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'textInput',
            name: 'gateway6',
            label: gettext('Gateway'),
            value: '',
            validators: {
              patternType: 'ipv6'
            },
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'method6' }, arg1: 'static' }
              }
            ],
            flex: 75
          },
          {
            type: 'numberInput',
            name: 'routemetric6',
            label: gettext('Metric'),
            value: 1,
            validators: {
              min: 0,
              max: 65535,
              patternType: 'integer'
            },
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'ne', arg0: { prop: 'method6' }, arg1: 'static' }
              }
            ]
          }
        ]
      },
      {
        type: 'divider',
        title: gettext('Advanced settings')
      },
      {
        type: 'textInput',
        name: 'dnsnameservers',
        label: gettext('DNS servers'),
        hint: gettext('IP addresses of domain name servers used to resolve host names.'),
        value: '',
        validators: {
          patternType: 'ipList'
        }
      },
      {
        type: 'textInput',
        name: 'dnssearch',
        label: gettext('Search domains'),
        hint: gettext('Domains used when resolving host names.'),
        value: '',
        validators: {
          patternType: 'domainNameList'
        }
      },
      {
        type: 'numberInput',
        name: 'mtu',
        label: gettext('MTU'),
        hint: gettext(
          'The maximum transmission unit in bytes to set for the device. Set to 0 to use the default value.'
        ),
        value: 0,
        validators: {
          min: 0,
          max: 65535,
          patternType: 'integer'
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/network/interfaces'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/network/interfaces'
        }
      }
    ]
  };
}
