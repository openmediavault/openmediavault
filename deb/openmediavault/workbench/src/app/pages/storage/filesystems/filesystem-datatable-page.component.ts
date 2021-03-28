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
        prop: 'devicefile'
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
          warningThreshold: 85
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
        template: 'create',
        execute: {
          type: 'url',
          url: '/storage/filesystems/create'
        }
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
        type: 'iconButton',
        icon: 'start',
        tooltip: gettext('Mount'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          constraint: [
            // Enable button if file systems is online.
            { operator: 'eq', arg0: { prop: 'status' }, arg1: 1 },
            // Enable button if file system supports fstab
            // mount entries.
            { operator: 'truthy', arg0: { prop: 'propfstab' } },
            // Enable button if the file system is not mounted.
            { operator: 'falsy', arg0: { prop: 'mounted' } }
          ]
        },
        confirmationDialogConfig: {
          template: 'confirmation',
          message: gettext('Do you really want to mount the file system?')
        },
        execute: {
          type: 'request',
          request: {
            service: 'FileSystemMgmt',
            method: 'mount',
            params: {
              id: '{{ _selected[0].devicefile }}',
              fstab: true
            },
            progressMessage: gettext('Please wait, the file system is being mounted ...')
          }
        }
      },
      {
        type: 'iconButton',
        icon: 'eject',
        tooltip: gettext('Unmount'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          constraint: [
            // Enable button if file system is not in use.
            {
              operator: 'if',
              arg0: { operator: 'has', arg0: { prop: '_used' } },
              arg1: { operator: 'falsy', arg0: { prop: '_used' } }
            },
            // Enable button if file system supports fstab
            // mount entries.
            { operator: 'truthy', arg0: { prop: 'propfstab' } },
            // Enable button if the file system is mounted.
            { operator: 'truthy', arg0: { prop: 'mounted' } }
          ]
        },
        confirmationDialogConfig: {
          template: 'confirmation-danger',
          message: gettext('Do you really want to unmount the file system?')
        },
        execute: {
          type: 'request',
          request: {
            service: 'FileSystemMgmt',
            method: 'umount',
            params: {
              id: '{{ _selected[0].devicefile }}',
              fstab: true
            }
          }
        }
      },
      {
        template: 'delete',
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
          template: 'confirmation-danger',
          message: gettext('Do you really want to delete the file system? All data will be lost.')
        },
        execute: {
          type: 'request',
          request: {
            service: 'FileSystemMgmt',
            method: 'delete',
            params: {
              id: '{{ _selected[0].devicefile }}'
            }
          }
        }
      }
    ]
  };
}
