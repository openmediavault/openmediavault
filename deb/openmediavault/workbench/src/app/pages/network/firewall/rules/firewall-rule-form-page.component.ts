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
export class FirewallRuleFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Iptables',
      get: {
        method: 'getRule',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setRule'
      }
    },
    fields: [
      /* eslint-disable max-len */
      {
        type: 'confObjUuid'
      },
      {
        type: 'hidden',
        name: 'rulenum',
        value: -1
      },
      {
        type: 'select',
        name: 'family',
        label: gettext('Family'),
        disabled: true,
        value: '{{ _routeParams.family }}',
        store: {
          data: [
            ['inet', 'IPv4'],
            ['inet6', 'IPv6']
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'select',
        name: 'chain',
        label: gettext('Direction'),
        value: 'INPUT',
        store: {
          data: [
            ['INPUT', 'INPUT'],
            ['OUTPUT', 'OUTPUT']
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'select',
        name: 'action',
        label: gettext('Action'),
        hint: gettext('This specifies what to do if the packet matches.'),
        value: 'REJECT',
        store: {
          data: [
            ['ACCEPT', 'ACCEPT'],
            ['REJECT', 'REJECT'],
            ['DROP', 'DROP'],
            ['LOG', 'LOG'],
            ['', gettext('Nothing')]
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'source',
        label: gettext('Source'),
        hint: gettext(
          'Source address can be either a network IP address (with /mask), a IP range or a plain IP address. A "!" argument before the address specification inverts the sense of the address.'
        ),
        value: ''
      },
      {
        type: 'textInput',
        name: 'sport',
        label: gettext('Source port'),
        hint: gettext(
          'Match if the source port is one of the given ports, e.g. 21 or !443 or 1024-65535.'
        ),
        value: '',
        validators: {
          custom: [
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'notEmpty',
                  arg0: { prop: 'sport' }
                },
                arg1: {
                  operator: 'regex',
                  arg0: {
                    prop: 'sport'
                  },
                  arg1: '^!?(([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])([-:]([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])){0,1})$'
                }
              },
              errorData: gettext(
                'This field should be a port or port range (e.g. 21 or !443 or 1024-65535).'
              )
            }
          ]
        }
      },
      {
        type: 'textInput',
        name: 'destination',
        label: gettext('Destination'),
        hint: gettext(
          'Destination address can be either a network IP address (with /mask), a IP range or a plain IP address. A "!" argument before the address specification inverts the sense of the address.'
        ),
        value: ''
      },
      {
        type: 'textInput',
        name: 'dport',
        label: gettext('Destination port'),
        hint: gettext(
          'Match if the destination port is one of the given ports, e.g. 21 or !443 or 1024-65535.'
        ),
        value: '',
        validators: {
          custom: [
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'notEmpty',
                  arg0: { prop: 'dport' }
                },
                arg1: {
                  operator: 'regex',
                  arg0: {
                    prop: 'dport'
                  },
                  arg1: '^!?(([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])([-:]([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])){0,1})$'
                }
              },
              errorData: gettext(
                'This field should be a port or port range (e.g. 21 or !443 or 1024-65535).'
              )
            }
          ]
        }
      },
      {
        type: 'select',
        name: 'protocol',
        label: gettext('Protocol'),
        value: 'tcp',
        store: {
          data: [
            ['tcp', 'TCP'],
            ['udp', 'UDP'],
            ['icmp', 'ICMP'],
            ['icmpv6', 'ICMPv6'],
            ['all', gettext('All')],
            ['!tcp', gettext('Not TCP')],
            ['!udp', gettext('Not UDP')],
            ['!icmp', gettext('Not ICMP')],
            ['!icmpv6', gettext('Not ICMPv6')]
          ]
        },
        validators: {
          custom: [
            // The protocol 'All' is not supported in conjunction with
            // the source/destination port option.
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'or',
                  arg0: {
                    operator: 'notEmpty',
                    arg0: { prop: 'sport' }
                  },
                  arg1: {
                    operator: 'notEmpty',
                    arg0: { prop: 'dport' }
                  }
                },
                arg1: {
                  operator: 'ne',
                  arg0: { prop: 'protocol' },
                  arg1: 'all'
                }
              },
              errorData: gettext(
                'The protocol is not supported in conjunction with a source/destination port.'
              )
            }
          ]
        }
      },
      {
        type: 'textInput',
        name: 'extraoptions',
        label: gettext('Extra options'),
        value: ''
      },
      {
        type: 'tagInput',
        name: 'comment',
        label: gettext('Tags'),
        value: ''
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/network/firewall/rules/{{ _routeParams.family }}'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/network/firewall/rules/{{ _routeParams.family }}'
        }
      }
    ]
  };
}
