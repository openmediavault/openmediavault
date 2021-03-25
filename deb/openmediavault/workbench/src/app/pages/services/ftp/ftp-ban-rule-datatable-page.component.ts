import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class FtpBanRuleDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '5cb9dfa8-36fd-11ea-99ec-5ba8c267c020',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    columns: [
      { name: gettext('Event'), prop: 'event', flexGrow: 1, sortable: true },
      { name: gettext('Occurrence'), prop: 'occurrence', flexGrow: 1, sortable: true },
      { name: gettext('Time interval'), prop: 'timeinterval', flexGrow: 1, sortable: true },
      { name: gettext('Expire'), prop: 'expire', flexGrow: 1, sortable: true }
    ],
    sorters: [
      {
        dir: 'asc',
        prop: 'event'
      }
    ],
    store: {
      proxy: {
        service: 'FTP',
        get: {
          method: 'getModBanRuleList'
        }
      }
    },
    actions: [
      {
        template: 'create',
        execute: {
          type: 'url',
          url: '/services/ftp/ban-rules/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/services/ftp/ban-rules/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'FTP',
            method: 'deleteModBanRule',
            params: {
              uuid: '{{ _selected[0].uuid }}'
            }
          }
        }
      }
    ]
  };
}
