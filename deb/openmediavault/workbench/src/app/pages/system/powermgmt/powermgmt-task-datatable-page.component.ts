import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
})
export class PowermgmtTaskDatatablePageComponent {
  public config: DatatablePageConfig = {
    autoReload: false,
    stateId: '49d0980c-2594-11ea-a805-bfcb34ee0962',
    sorters: [
      {
        dir: 'asc',
        prop: 'enable'
      }
    ],
    store: {
      proxy: {
        service: 'PowerMgmt',
        get: {
          method: 'getScheduleList',
          params: {
            type: ['reboot', 'shutdown', 'standby']
          }
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
        name: gettext('Type'),
        prop: 'type',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          map: {
            reboot: { value: gettext('Reboot') },
            standby: { value: gettext('Standby') },
            shutdown: { value: gettext('Shutdown') }
          }
        }
      },
      {
        name: gettext('Scheduling'),
        prop: '',
        flexGrow: 1,
        cellTemplateName: 'template',
        cellTemplateConfig:
          '{% if execution == "exactly" %}' +
          '{% set _minute = minute %}' +
          '{% set _hour = hour %}' +
          '{% set _dayofmonth = dayofmonth %}' +
          '{% if everynminute %}{% set _minute %}*/{{ minute }}{% endset %}{% endif %}' +
          '{% if everynhour %}{% set _hour %}*/{{ hour }}{% endset %}{% endif %}' +
          '{% if everyndayofmonth %}{% set _dayofmonth %}*/{{ dayofmonth }}{% endset %}{% endif %}' +
          '{{ _minute }} {{ _hour }} {{ _dayofmonth }} {{ month }} {{ dayofweek }}' +
          '{% else %}' +
          '{{ execution | capitalize | translate }}' +
          '{% endif %}'
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
          url: '/system/powermgmt/tasks/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/system/powermgmt/tasks/edit/{{ uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'PowerMgmt',
            method: 'deleteScheduledJob',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };
}
