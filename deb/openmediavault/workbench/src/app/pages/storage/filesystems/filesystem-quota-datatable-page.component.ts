import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class FilesystemQuotaDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '3f8c62be-11ba-4d37-8683-cbe44c049955',
    autoReload: false,
    remoteSorting: false,
    remotePaging: false,
    sorters: [
      {
        dir: 'asc',
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'Quota',
        get: {
          method: 'get',
          params: {
            uuid: '{{ _routeParams.uuid }}'
          }
        }
      }
    },
    columns: [
      {
        name: gettext('Type'),
        prop: 'type',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          map: {
            user: { value: gettext('User') },
            group: { value: gettext('Group') }
          }
        }
      },
      { name: gettext('Name'), prop: 'name', flexGrow: 1, sortable: true },
      {
        name: gettext('Used capacity'),
        prop: 'bused',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Quota'),
        prop: 'bhardlimit',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'template',
        cellTemplateConfig: '{{ bhardlimit }} {{ bunit }}'
      }
    ],
    actions: [
      {
        template: 'edit',
        execute: {
          type: 'url',
          url:
            '/storage/filesystems/quota/{{ _routeParams.uuid }}/edit/{{ _selected[0].type }}/{{ _selected[0].name }}'
        }
      }
    ],
    buttons: [
      {
        text: gettext('Back'),
        url: '/storage/filesystems'
      }
    ]
  };
}
