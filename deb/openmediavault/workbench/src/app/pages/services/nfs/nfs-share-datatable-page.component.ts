import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class NfsShareDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '4aa1756c-c761-403d-91fe-d67985cd8133',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    columns: [
      { name: gettext('Shared folder'), prop: 'sharedfoldername', flexGrow: 1, sortable: true },
      { name: gettext('Client'), prop: 'client', flexGrow: 1, sortable: true },
      {
        name: gettext('Options'),
        prop: 'options',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'template',
        cellTemplateConfig:
          '{{ extraoptions | split(",") | union(options | split(",")) | uniq() | sort() | join(", ") }}'
      },
      { name: gettext('Comment'), prop: 'comment', flexGrow: 1, sortable: true }
    ],
    sorters: [
      {
        dir: 'asc',
        prop: 'sharedfoldername'
      }
    ],
    store: {
      proxy: {
        service: 'NFS',
        get: {
          method: 'getShareList'
        }
      }
    },
    actions: [
      {
        template: 'create',
        execute: {
          type: 'url',
          url: '/services/nfs/shares/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/services/nfs/shares/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'NFS',
            method: 'deleteShare',
            params: {
              uuid: '{{ _selected[0].uuid }}'
            }
          }
        }
      }
    ]
  };
}
