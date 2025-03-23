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
                service: 'Smart',
                method: 'executeScheduledTest',
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
