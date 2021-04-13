import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class SmartTaskDatatablePageComponent {
  public config: DatatablePageConfig = {
    autoReload: false,
    stateId: '97ec21be-4d8a-11ea-b4b9-e33289777918',
    sorters: [
      {
        dir: 'asc',
        prop: 'devicefile'
      }
    ],
    store: {
      proxy: {
        service: 'Smart',
        get: {
          method: 'getScheduleList'
        }
      }
    },
    columns: [
      {
        name: gettext('Enabled'),
        prop: 'enable',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      {
        name: gettext('Device'),
        prop: 'devicefile',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Type'),
        prop: 'type',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          map: {
            /* eslint-disable @typescript-eslint/naming-convention */
            S: { value: gettext('Short self-test') },
            L: { value: gettext('Long self-test') },
            C: { value: gettext('Conveyance self-test') },
            O: { value: gettext('Offline immediate test') }
          }
        }
      },
      {
        name: gettext('Scheduling'),
        prop: '',
        flexGrow: 1,
        cellTemplateName: 'template',
        cellTemplateConfig: '{{ hour }} {{ dayofmonth }} {{ month }} {{ dayofweek }}'
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
          url: '/storage/smart/tasks/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/storage/smart/tasks/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'Smart',
            method: 'deleteScheduledTest',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };
}
