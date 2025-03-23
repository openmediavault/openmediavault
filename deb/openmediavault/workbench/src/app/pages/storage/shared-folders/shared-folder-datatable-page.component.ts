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
export class SharedFolderDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: 'c0a05d92-2d72-11ea-9b29-33dda9c523cc',
    autoReload: false,
    hasSearchField: true,
    remoteSorting: true,
    remotePaging: true,
    sorters: [
      {
        dir: 'asc',
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'ShareMgmt',
        get: {
          method: 'getList'
        }
      },
      transform: {
        absdirpath: '/{{ [mntent.dir, reldirpath] | map("strip", "/")  | compact() | join("/") }}'
      }
    },
    rowEnumFmt: '{{ name }}',
    columns: [
      {
        name: gettext('Name'),
        prop: 'name',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Device'),
        prop: 'device',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Relative Path'),
        prop: 'reldirpath',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Absolute Path'),
        prop: 'absdirpath',
        flexGrow: 2,
        sortable: true,
        cellTemplateName: 'copyToClipboard'
      },
      {
        name: gettext('Referenced'),
        prop: '_used',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
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
    actions: [
      {
        template: 'create',
        execute: {
          type: 'url',
          url: '/storage/shared-folders/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/storage/shared-folders/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'mdi:folder-key',
        tooltip: gettext('Permissions'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'url',
          url: '/storage/shared-folders/permissions/{{ _selected[0].uuid }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'mdi:format-list-checks',
        tooltip: gettext('Access control list'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          constraint: [
            // Enable button if the selected shared folder is located on
            // a POSIX compliant file system.
            { operator: 'truthy', arg0: { prop: 'mntent.posixacl' } }
          ]
        },
        execute: {
          type: 'url',
          url: '/storage/shared-folders/acl/{{ _selected[0].uuid }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'mdi:camera',
        tooltip: gettext('Snapshots'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          constraint: [{ operator: 'truthy', arg0: { prop: 'snapshots' } }]
        },
        execute: {
          type: 'url',
          url: '/storage/shared-folders/snapshots/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        enabledConstraints: {
          constraint: [
            // Disable button if a selected shared folder is in use.
            { operator: 'falsy', arg0: { prop: '_used' } }
          ]
        },
        execute: {
          type: 'request',
          request: {
            service: 'ShareMgmt',
            method: 'delete',
            params: {
              uuid: '{{ uuid }}',
              recursive: false
            }
          }
        }
      },
      {
        type: 'divider'
      },
      {
        type: 'iconButton',
        icon: 'mdi:camera',
        tooltip: gettext('All Snapshots'),
        execute: {
          type: 'url',
          url: '/storage/shared-folders/snapshots'
        }
      }
    ]
  };
}
