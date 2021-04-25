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
import * as _ from 'lodash';

import { DatatablePageActionConfig } from '~/app/core/components/limn-ui/models/datatable-page-action-config.type';
import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { DataStore } from '~/app/shared/models/data-store.type';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class FirewallRuleInetDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '9cee722c-7c04-11ea-a3e8-37671db9f618',
    autoReload: false,
    limit: 0,
    remoteSorting: false,
    remotePaging: false,
    hasSearchField: false,
    columns: [
      {
        name: gettext('Direction'),
        prop: 'chain',
        flexGrow: 1,
        sortable: false,
        cellTemplateName: 'chip'
      },
      {
        name: gettext('Action'),
        prop: 'action',
        flexGrow: 1,
        sortable: false,
        cellTemplateName: 'chip'
      },
      { name: gettext('Source'), prop: 'source', flexGrow: 1, sortable: false },
      { name: gettext('Port'), prop: 'sport', flexGrow: 1, sortable: false },
      { name: gettext('Destination'), prop: 'destination', flexGrow: 1, sortable: false },
      { name: gettext('Port'), prop: 'dport', flexGrow: 1, sortable: false },
      {
        name: gettext('Protocol'),
        prop: 'protocol',
        flexGrow: 1,
        sortable: false,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          map: {
            tcp: { value: 'TCP' },
            udp: { value: 'UDP' },
            icmp: { value: 'ICMP' },
            icmpv6: { value: 'ICMPv6' },
            all: { value: gettext('All') },
            '!tcp': { value: gettext('Not TCP') },
            '!udp': { value: gettext('Not UDP') },
            '!icmp': { value: gettext('Not ICMP') },
            '!icmpv6': { value: gettext('Not ICMPv6') }
          }
        }
      },
      { name: gettext('Extra options'), prop: 'extraoptions', flexGrow: 1, sortable: false },
      { name: gettext('Comment'), prop: 'comment', flexGrow: 1, sortable: false }
    ],
    sorters: [
      {
        dir: 'asc',
        prop: 'rulenum'
      }
    ],
    store: {
      proxy: {
        service: 'Iptables',
        get: {
          method: 'getRules',
          params: {
            type: ['userdefined']
          }
        }
      }
    },
    actions: [
      {
        type: 'iconButton',
        icon: 'save',
        tooltip: gettext('Save'),
        click: this.onSave.bind(this),
        enabledConstraints: {
          callback: (selected, store: DataStore) => _.get(store, 'dirty', false)
        }
      },
      {
        type: 'divider'
      },
      {
        template: 'add',
        execute: {
          type: 'url',
          url: '/network/firewall/rules/inet/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/network/firewall/rules/inet/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'arrowUp',
        tooltip: gettext('Up'),
        click: this.onUp.bind(this),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        }
      },
      {
        type: 'iconButton',
        icon: 'arrowDown',
        tooltip: gettext('Down'),
        click: this.onDown.bind(this),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'Iptables',
            method: 'deleteRule',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };

  constructor(private rpcService: RpcService, private notificationService: NotificationService) {}

  onSave(action: DatatablePageActionConfig, selection: DatatableSelection, store: DataStore) {
    this.rpcService.request('Iptables', 'setRules', store.data).subscribe(() => {
      _.set(store, 'dirty', false);
      this.notificationService.show(NotificationType.success, gettext('Updated firewall rules.'));
    });
  }

  onUp(action: DatatablePageActionConfig, selection: DatatableSelection, store: DataStore) {
    const selected = selection.first();
    const index = _.findIndex(store.data, selected);
    if (index <= 0) {
      return;
    }
    // Create a working copy.
    const clone = _.cloneDeep(store.data);
    // Relocate rule.
    _.pullAt(clone, index);
    clone.splice(index - 1, 0, selected);
    this.updateRuleNumbers(clone);
    // Update the store data. The datatable will be updated automatically.
    store.data = clone;
    // Mark the data as dirty.
    _.set(store, 'dirty', true);
  }

  onDown(action: DatatablePageActionConfig, selection: DatatableSelection, store: DataStore) {
    const selected = selection.first();
    const index = _.findIndex(store.data, selected);
    if (index + 1 >= store.data.length) {
      return;
    }
    // Create a working copy.
    const clone = _.cloneDeep(store.data);
    // Relocate rule.
    _.pullAt(clone, index);
    clone.splice(index + 1, 0, selected);
    this.updateRuleNumbers(clone);
    // Update the store data. The datatable will be updated automatically.
    store.data = clone;
    // Mark the data as dirty.
    _.set(store, 'dirty', true);
  }

  private updateRuleNumbers(rules: Array<any>) {
    _.forEach(rules, (value, key) => {
      _.set(value, 'rulenum', key);
    });
  }
}
