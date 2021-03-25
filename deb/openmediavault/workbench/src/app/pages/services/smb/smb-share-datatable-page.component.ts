import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class SmbShareDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '14ee24b4-330a-11ea-b3fb-eb6c148bc874',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    columns: [
      {
        name: gettext('Enabled'),
        prop: 'enable',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      { name: gettext('Shared folder'), prop: 'sharedfoldername', flexGrow: 1, sortable: true },
      { name: gettext('Comment'), prop: 'comment', flexGrow: 1, sortable: true },
      {
        name: gettext('Public'),
        prop: 'guest',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          map: {
            no: { value: gettext('No') },
            allow: { value: gettext('Guests allowed') },
            only: { value: gettext('Guests only') }
          }
        }
      },
      {
        name: gettext('Read-only'),
        prop: 'readonly',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      {
        name: gettext('Browseable'),
        prop: 'browseable',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      }
    ],
    sorters: [
      {
        dir: 'asc',
        prop: 'sharedfoldername'
      }
    ],
    store: {
      proxy: {
        service: 'SMB',
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
          url: '/services/smb/shares/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/services/smb/shares/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'SMB',
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
