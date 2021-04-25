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

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class FilesystemDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '66d9d2ce-2fee-11ea-8386-e3eba0cf8f78',
    autoReload: 10000,
    remoteSorting: true,
    remotePaging: true,
    sorters: [
      {
        dir: 'asc',
        prop: 'canonicaldevicefile'
      }
    ],
    store: {
      proxy: {
        service: 'FileSystemMgmt',
        get: {
          method: 'getListBg',
          task: true
        }
      }
    },
    rowId: 'devicefile',
    columns: [
      {
        name: gettext('Device'),
        prop: 'canonicaldevicefile',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Comment'),
        prop: 'comment',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Device(s)'),
        prop: 'devicefiles',
        flexGrow: 2,
        sortable: false,
        hidden: true,
        cellTemplateName: 'unsortedList'
      },
      {
        name: gettext('Identify As'),
        prop: '',
        flexGrow: 1,
        cellTemplateName: 'template',
        hidden: true,
        cellTemplateConfig: '{% if uuid %}UUID={{ uuid }}{% else %}{{ devicefile }}{% endif %}'
      },
      {
        name: gettext('Label'),
        prop: 'label',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Parent Device'),
        prop: 'parentdevicefile',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Type'),
        prop: 'type',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          template: '{{ type | upper }}'
        }
      },
      {
        name: gettext('Total'),
        prop: 'size',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'template',
        cellTemplateConfig: '{{ size | tobytes | binaryunit | notavailable("-") }}'
      },
      {
        name: gettext('Available'),
        prop: 'available',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'template',
        cellTemplateConfig: '{{ available | tobytes | binaryunit | notavailable("-") }}'
      },
      {
        name: gettext('Used'),
        prop: 'percentage',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'progressBar',
        cellTemplateConfig: {
          text: '{{ used | tobytes | binaryunit | notavailable("-") }}',
          warningThreshold: '{{ usagewarnthreshold | default(0) }}'
        }
      },
      {
        name: gettext('Mounted'),
        prop: 'mounted',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      {
        name: gettext('Mount Point'),
        prop: 'mountpoint',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Referenced'),
        prop: '_used',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      {
        name: gettext('Status'),
        prop: 'status',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          map: {
            1: { value: gettext('Online'), class: 'omv-chip-theme-success' },
            2: { value: gettext('Initializing'), class: 'omv-chip-theme-info' },
            3: { value: gettext('Missing'), class: 'omv-chip-theme-error' }
          }
        }
      }
    ],
    actions: [
      {
        type: 'menu',
        icon: 'add',
        tooltip: gettext('Create | Mount'),
        actions: [
          {
            template: 'create',
            execute: {
              type: 'url',
              url: '/storage/filesystems/create'
            }
          },
          {
            type: 'iconButton',
            text: gettext('Mount'),
            icon: 'start',
            execute: {
              type: 'url',
              url: '/storage/filesystems/mount'
            }
          }
        ]
      },
      {
        type: 'iconButton',
        icon: 'expand',
        tooltip: gettext('Resize'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          constraint: [
            // Enable button if the file system is online.
            { operator: 'eq', arg0: { prop: 'status' }, arg1: 1 },
            // Enable button if the file system supports online resizing.
            { operator: 'truthy', arg0: { prop: 'propresize' } },
            // Enable button if the file system is not read-only.
            { operator: 'falsy', arg0: { prop: '_readonly' } }
          ]
        },
        confirmationDialogConfig: {
          template: 'confirmation',
          message: gettext(
            'Do you really want to resize the file system? You have to do that after a RAID has been grown for example.'
          )
        },
        execute: {
          type: 'request',
          request: {
            service: 'FileSystemMgmt',
            method: 'resize',
            params: {
              id: '{{ _selected[0].uuid }}'
            }
          }
        }
      },
      {
        type: 'iconButton',
        icon: 'mdi:chart-pie',
        tooltip: gettext('Quota'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          constraint: [
            // Enable button if file systems is online.
            { operator: 'eq', arg0: { prop: 'status' }, arg1: 1 },
            // Enable button if file system supports quota.
            { operator: 'truthy', arg0: { prop: 'propquota' } },
            // Enable button if the file system is mounted.
            { operator: 'truthy', arg0: { prop: 'mounted' } },
            // Enable button if the file system is not read-only.
            { operator: 'falsy', arg0: { prop: '_readonly' } }
          ]
        },
        execute: {
          type: 'url',
          url: '/storage/filesystems/quota/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        icon: 'stop',
        tooltip: gettext('Unmount'),
        enabledConstraints: {
          constraint: [
            // Disable button if file system is in use or read-only.
            {
              operator: 'if',
              arg0: { operator: 'has', arg0: { prop: '_used' } },
              arg1: { operator: 'falsy', arg0: { prop: '_used' } }
            },
            {
              operator: 'if',
              arg0: { operator: 'has', arg0: { prop: '_readonly' } },
              arg1: { operator: 'falsy', arg0: { prop: '_readonly' } }
            },
            // Disable button if file system is initialized (status=2)
            // at the moment.
            { operator: 'ne', arg0: { prop: 'status' }, arg1: 2 },
            // Enable button if file system supports fstab
            // mount entries.
            { operator: 'truthy', arg0: { prop: 'propfstab' } }
          ]
        },
        confirmationDialogConfig: {
          title: gettext('Unmount'),
          template: 'confirmation',
          message: gettext(
            // eslint-disable-next-line max-len
            'Do you really want to unmount this file system? Please make sure that the file system is not used by any service before unmounting. Note, the file system will not be deleted by this action.'
          )
        },
        execute: {
          type: 'request',
          request: {
            service: 'FileSystemMgmt',
            method: 'umount',
            params: {
              id: '{{ devicefile }}',
              fstab: true
            },
            progressMessage: gettext('Please wait, unmounting the file system ...')
          }
        }
      }
    ]
  };
}
