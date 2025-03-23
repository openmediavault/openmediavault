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
export class SmartDeviceDatatablePageComponent {
  public config: DatatablePageConfig = {
    autoReload: false,
    stateId: '453b944e-4d7c-11ea-b897-2bd8eeecbd33',
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
          method: 'getListBg',
          task: true
        }
      },
      transform: {
        temperature: '{% if temperature %}{{ temperature }} °C{% endif %}'
      }
    },
    columns: [
      {
        name: gettext('Monitored'),
        prop: 'monitor',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      {
        name: gettext('Device'),
        prop: 'canonicaldevicefile',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Device Symlinks'),
        prop: 'devicelinks',
        flexGrow: 2,
        sortable: false,
        hidden: true,
        cellTemplateName: 'unsortedList'
      },
      {
        name: gettext('Model'),
        prop: 'model',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Vendor'),
        prop: 'vendor',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Serial Number'),
        prop: 'serialnumber',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('WWN'),
        prop: 'wwn',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Capacity'),
        prop: 'size',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'binaryUnit'
      },
      {
        name: gettext('Temperature'),
        prop: 'temperature',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Status'),
        prop: 'overallstatus',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          /* eslint-disable @typescript-eslint/naming-convention */
          map: {
            GOOD: { value: gettext('Good'), class: 'omv-background-color-pair-success' },
            BAD_STATUS: { value: gettext('Unknown') },
            BAD_ATTRIBUTE_NOW: {
              value: gettext('Bad'),
              class: 'omv-background-color-pair-error',
              tooltip: gettext('Device is being used outside design parameters.')
            },
            BAD_ATTRIBUTE_IN_THE_PAST: {
              value: gettext('Bad'),
              class: 'omv-background-color-pair-error',
              tooltip: gettext('Device was used outside of design parameters in the past.')
            },
            BAD_SECTOR: {
              value: gettext('Warning'),
              class: 'omv-background-color-pair-warning',
              tooltip: gettext('Device has a few bad sectors.')
            },
            BAD_SECTOR_MANY: {
              value: gettext('Bad'),
              class: 'omv-background-color-pair-error',
              tooltip: gettext('Device has many bad sectors.')
            }
          }
        }
      }
    ],
    actions: [
      {
        template: 'edit',
        execute: {
          type: 'url',
          url:
            // eslint-disable-next-line max-len
            '/storage/smart/devices/{{ "create" if _selected[0].uuid == newconfobjuuid else "edit" }}/{{ _selected[0].uuid }}/{{ _selected[0].devicefile | encodeuricomponent }}'
        }
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
          url: '/storage/smart/devices/details/{{ _selected[0].devicefile | encodeuricomponent }}'
        }
      }
    ]
  };
}
