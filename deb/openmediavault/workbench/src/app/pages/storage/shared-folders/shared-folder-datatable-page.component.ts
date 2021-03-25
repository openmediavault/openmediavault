import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class SharedFolderDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: 'c0a05d92-2d72-11ea-9b29-33dda9c523cc',
    autoReload: false,
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
      }
    },
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
        cellTemplateName: 'template',
        cellTemplateConfig:
          '/{{ [mntent.dir, reldirpath] | map("strip", "/")  | compact() | join("/") }}'
      },
      {
        name: gettext('Referenced'),
        prop: '_used',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      {
        name: gettext('Comment'),
        prop: 'comment',
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
        icon: 'mdi:account-check',
        tooltip: gettext('Privileges'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'url',
          url: '/storage/shared-folders/privileges/{{ _selected[0].uuid }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'mdi:format-list-checks',
        tooltip: gettext('Access control list'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'url',
          url: '/storage/shared-folders/acl/{{ _selected[0].uuid }}'
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
      }
    ]
  };
}
