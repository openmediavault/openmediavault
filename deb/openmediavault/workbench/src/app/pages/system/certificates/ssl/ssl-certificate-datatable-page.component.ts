import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class SslCertificateDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '9fe3e818-1c32-11ea-bde4-5f0ce98b6927',
    columns: [{ name: gettext('Comment'), prop: 'comment', flexGrow: 1, sortable: true }],
    sorters: [
      {
        dir: 'asc',
        prop: 'comment'
      }
    ],
    store: {
      proxy: {
        service: 'CertificateMgmt',
        get: {
          method: 'getList'
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
              url: '/system/certificate/ssl/create'
            }
          },
          {
            type: 'iconButton',
            text: gettext('Import'),
            icon: 'import',
            execute: {
              type: 'url',
              url: '/system/certificate/ssl/import'
            }
          }
        ]
      },
      {
        type: 'iconButton',
        icon: 'details',
        tooltip: gettext('Show details'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'url',
          url: '/system/certificate/ssl/detail/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'CertificateMgmt',
            method: 'delete',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };
}
