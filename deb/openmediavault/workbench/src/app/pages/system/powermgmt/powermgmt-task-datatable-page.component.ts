/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import { Component } from '@angular/core';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';

@Component({
  template: '<omv-intuition-datatable-page [config]="this.config"></omv-intuition-datatable-page>'
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
          '{{ [_minute, _hour, _dayofmonth, month, dayofweek] | join(" ") | cron2human }}' +
          '{% else %}' +
          '{{ execution | capitalize | translate }}' +
          '{% endif %}'
      },
      {
        name: gettext('Tags'),
        prop: 'comment',
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          separator: ','
        },
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
          url: '/system/powermgmt/tasks/edit/{{ _selected[0].uuid }}'
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
