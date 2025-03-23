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
export class CronTaskDatatablePageComponent {
  public config: DatatablePageConfig = {
    autoReload: false,
    stateId: '43b8eb04-2594-11ea-8bd6-fb7a107fca0e',
    sorters: [
      {
        dir: 'asc',
        prop: 'enable'
      }
    ],
    store: {
      proxy: {
        service: 'Cron',
        get: {
          method: 'getList',
          params: {
            type: ['userdefined']
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
        name: gettext('User'),
        prop: 'username',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Command'),
        prop: 'command',
        cellTemplateName: 'text',
        flexGrow: 1,
        sortable: true
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
          url: '/system/cron/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/system/cron/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'start',
        tooltip: gettext('Run'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'taskDialog',
          taskDialog: {
            config: {
              title: gettext('Run scheduled task'),
              startOnInit: true,
              request: {
                service: 'Cron',
                method: 'execute',
                params: {
                  uuid: '{{ _selected[0].uuid }}'
                }
              }
            }
          }
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'Cron',
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
