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

import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';

@Component({
  template: '<omv-intuition-datatable-page [config]="this.config"></omv-intuition-datatable-page>'
})
export class GroupDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '6846696d-6834-45b8-9154-423488469072',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    remoteSearching: true,
    hasSearchField: true,
    rowId: 'name',
    rowEnumFmt: '{{ name }}',
    columns: [
      { name: gettext('Name'), prop: 'name', flexGrow: 1, sortable: true },
      {
        name: gettext('GID'),
        prop: 'gid',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Members'),
        prop: 'members',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'template',
        cellTemplateConfig: '{{ members | sort() | join(", ") }}'
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
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'UserMgmt',
        get: {
          method: 'getGroupList'
        }
      }
    },
    actions: [
      {
        type: 'menu',
        icon: 'add',
        tooltip: gettext('Create | Import'),
        actions: [
          {
            template: 'create',
            execute: {
              type: 'url',
              url: '/usermgmt/groups/create'
            }
          },
          {
            type: 'iconButton',
            text: gettext('Import'),
            icon: 'import',
            execute: {
              type: 'url',
              url: '/usermgmt/groups/import'
            }
          }
        ]
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/usermgmt/groups/edit/{{ _selected[0].name }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'mdi:folder-key',
        tooltip: gettext('Shared folder permissions'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'url',
          url: '/usermgmt/groups/permissions/{{ _selected[0].name }}'
        }
      },
      {
        template: 'delete',
        enabledConstraints: {
          minSelected: 1,
          constraint: [{ operator: 'z', arg0: { prop: 'members' } }]
        },
        execute: {
          type: 'request',
          request: {
            service: 'UserMgmt',
            method: 'deleteGroup',
            params: {
              name: '{{ name }}'
            }
          }
        }
      }
    ]
  };
}
