/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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

import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class ProcessesFormPageComponent {
  public config: FormPageConfig = {
    autoReload: 10000,
    request: {
      service: 'System',
      get: {
        method: 'getTopInfo',
        params: {
          format: 'json'
        }
      }
    },
    fields: [
      {
        type: 'container',
        fields: [
          {
            type: 'textInput',
            name: 'time',
            label: gettext('System Time'),
            value: '',
            readonly: true
          },
          {
            type: 'textInput',
            name: 'uptime',
            label: gettext('Uptime'),
            value: '',
            readonly: true
          },
          {
            type: 'textInput',
            name: 'users',
            label: gettext('Users'),
            value: '',
            readonly: true
          }
        ]
      },
      {
        type: 'hidden',
        name: 'load_1m'
      },
      {
        type: 'hidden',
        name: 'load_5m'
      },
      {
        type: 'hidden',
        name: 'load_15m'
      },
      {
        type: 'hidden',
        name: 'tasks_total'
      },
      {
        type: 'hidden',
        name: 'tasks_running'
      },
      {
        type: 'hidden',
        name: 'tasks_sleeping'
      },
      {
        type: 'hidden',
        name: 'tasks_stopped'
      },
      {
        type: 'hidden',
        name: 'tasks_zombie'
      },
      {
        type: 'hidden',
        name: 'mem_total'
      },
      {
        type: 'hidden',
        name: 'mem_free'
      },
      {
        type: 'hidden',
        name: 'mem_used'
      },
      {
        type: 'hidden',
        name: 'mem_buff_cache'
      },
      {
        type: 'hidden',
        name: 'swap_total'
      },
      {
        type: 'hidden',
        name: 'swap_free'
      },
      {
        type: 'hidden',
        name: 'swap_used'
      },
      {
        type: 'hidden',
        name: 'mem_available'
      },
      {
        type: 'container',
        fields: [
          {
            type: 'card',
            label: gettext('Load Average'),
            text: '1min:\t{{ load_1m }}<br>5min:\t{{ load_5m }}<br>15min:\t{{ load_15m }}'
          },
          {
            type: 'card',
            label: gettext('Tasks'),
            text: 'Total:\t\t{{ tasks_total }}<br>Running:\t\t{{ tasks_running }}<br>Sleeping:\t{{ tasks_sleeping }}<br>Stopped:\t{{ tasks_stopped }}<br>Zombie:\t\t{{ tasks_zombie }}'
          },
          {
            type: 'card',
            label: gettext('MiB Mem'),
            text: 'Total:\t\t{{ mem_total }}<br>Free:\t\t{{ mem_free }}<br>Used:\t\t{{ mem_used }}<br>Buff/Cache:\t{{ mem_buff_cache }}'
          },
          {
            type: 'card',
            label: gettext('MiB Swap'),
            text: 'Total:\t{{ swap_total }}<br>Free:\t{{ swap_free }}<br>Used:\t{{ swap_used }}<br>Avail:\t{{ mem_available }}'
          }
        ]
      },
      {
        type: 'datatable',
        name: 'processes',
        hasFooter: true,
        hasSearchField: true,
        limit: 0,
        stateId: '7b7fe8fe-5788-11f0-b8d6-b37d1ff753c3',
        selectionType: 'none',
        sorters: [
          {
            dir: 'desc',
            prop: 'percent_cpu'
          }
        ],
        columns: [
          {
            name: gettext('PID'),
            prop: 'PID',
            flexGrow: 1,
            sortable: true
          },
          {
            name: gettext('USER'),
            prop: 'USER',
            flexGrow: 1,
            sortable: true
          },
          {
            name: gettext('PR'),
            prop: 'PR',
            flexGrow: 1,
            sortable: true
          },
          {
            name: gettext('NI'),
            prop: 'NI',
            flexGrow: 1,
            sortable: true
          },
          {
            name: gettext('VIRT'),
            prop: 'VIRT',
            flexGrow: 1,
            sortable: true,
            cellTemplateName: 'binaryUnit'
          },
          {
            name: gettext('RES'),
            prop: 'RES',
            flexGrow: 1,
            sortable: true,
            cellTemplateName: 'binaryUnit'
          },
          {
            name: gettext('SHR'),
            prop: 'SHR',
            flexGrow: 1,
            sortable: true,
            cellTemplateName: 'binaryUnit'
          },
          {
            name: gettext('S'),
            prop: 'S',
            flexGrow: 1,
            sortable: true,
            cellTemplateName: 'chip',
            cellTemplateConfig: {
              /* eslint-disable @typescript-eslint/naming-convention */
              map: {
                D: { tooltip: gettext('Uninterruptible sleep') },
                I: { class: 'omv-background-color-pair-yellow', tooltip: gettext('Idle') },
                R: { class: 'omv-background-color-pair-green', tooltip: gettext('Running') },
                S: { class: 'omv-background-color-pair-orange', tooltip: gettext('Sleeping') },
                T: { tooltip: gettext('Stopped by job control signal') },
                t: { tooltip: gettext('Stopped by debugger during trace') },
                Z: { class: 'omv-background-color-pair-dark', tooltip: gettext('Zombie') }
              }
            }
          },
          {
            name: gettext('%CPU'),
            prop: '%CPU',
            flexGrow: 2,
            sortable: true,
            cellTemplateName: 'progressBar'
          },
          {
            name: gettext('%MEM'),
            prop: '%MEM',
            flexGrow: 2,
            sortable: true,
            cellTemplateName: 'progressBar'
          },
          {
            name: gettext('TIME+'),
            prop: 'TIME+',
            flexGrow: 1,
            sortable: true
          },
          {
            name: gettext('COMMAND'),
            prop: 'COMMAND',
            flexGrow: 3,
            sortable: true
          }
        ],
        value: []
      }
    ]
  };
}
