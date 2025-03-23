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
import * as _ from 'lodash';

import { DatatablePageActionConfig } from '~/app/core/components/intuition/models/datatable-page-action-config.type';
import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { BaseDatatablePageComponent } from '~/app/pages/base-page-component';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { Datatable } from '~/app/shared/models/datatable.interface';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  template: '<omv-intuition-datatable-page [config]="this.config"></omv-intuition-datatable-page>'
})
export class FirewallRuleInetDatatablePageComponent extends BaseDatatablePageComponent {
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
            /* eslint-disable @typescript-eslint/naming-convention */
            tcp: { value: 'TCP' },
            udp: { value: 'UDP' },
            icmp: { value: 'ICMP' },
            icmpv6: { value: 'ICMPv6' },
            all: { value: gettext('All') },
            '!tcp': { value: gettext('Not TCP') },
            '!udp': { value: gettext('Not UDP') },
            '!icmp': { value: gettext('Not ICMP') },
            '!icmpv6': { value: gettext('Not ICMPv6') }
            /* eslint-enable @typescript-eslint/naming-convention */
          }
        }
      },
      {
        name: gettext('Extra options'),
        prop: 'extraoptions',
        cellTemplateName: 'text',
        flexGrow: 1,
        sortable: false
      },
      {
        name: gettext('Tags'),
        prop: 'comment',
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          separator: ','
        },
        flexGrow: 1,
        sortable: true
      }
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
          callback: () => this.dirty
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

  constructor(
    private rpcService: RpcService,
    private notificationService: NotificationService
  ) {
    super();
  }

  onSave(action: DatatablePageActionConfig, table: Datatable) {
    this.rpcService.request('Iptables', 'setRules', table.data).subscribe(() => {
      this.dirty = false;
      this.notificationService.show(NotificationType.success, gettext('Updated firewall rules.'));
    });
  }

  onUp(action: DatatablePageActionConfig, table: Datatable) {
    const selected = table.selection.first();
    const index = _.findIndex(table.data, selected);
    if (index <= 0) {
      return;
    }
    // Create a working copy.
    const modifiedData = _.cloneDeep(table.data);
    // Relocate rule.
    _.pullAt(modifiedData, index);
    modifiedData.splice(index - 1, 0, selected);
    this.updateRuleNumbers(modifiedData);
    // Update the table data and redraw table content.
    table.updateData(modifiedData);
    // Mark the data as dirty.
    this.dirty = true;
  }

  onDown(action: DatatablePageActionConfig, table: Datatable) {
    const selected = table.selection.first();
    const index = _.findIndex(table.data, selected);
    if (index + 1 >= table.data.length) {
      return;
    }
    // Create a working copy.
    const modifiedData = _.cloneDeep(table.data);
    // Relocate rule.
    _.pullAt(modifiedData, index);
    modifiedData.splice(index + 1, 0, selected);
    this.updateRuleNumbers(modifiedData);
    // Update the table data and redraw table content.
    table.updateData(modifiedData);
    // Mark the data as dirty.
    this.dirty = true;
  }

  private updateRuleNumbers(rules: Array<any>) {
    _.forEach(rules, (value, key) => {
      _.set(value, 'rulenum', key);
    });
  }
}
