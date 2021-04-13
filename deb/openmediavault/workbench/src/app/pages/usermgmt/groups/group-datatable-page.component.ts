import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class GroupDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '6846696d-6834-45b8-9154-423488469072',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    hasSearchField: true,
    rowId: 'name',
    columns: [
      { name: gettext('Name'), prop: 'name', flexGrow: 1, sortable: true },
      {
        name: gettext('Members'),
        prop: 'members',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'template',
        cellTemplateConfig: '{{ members | sort() | join(", ") }}'
      },
      { name: gettext('Comment'), prop: 'comment', flexGrow: 1, sortable: true }
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
        tooltip: gettext('Shared folder privileges'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'url',
          url: '/usermgmt/groups/privileges/{{ _selected[0].name }}'
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
