/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';

@Component({
  template: '<omv-intuition-datatable-page [config]="this.config"></omv-intuition-datatable-page>'
})
export class MdDatatablePageComponent {
  public config: DatatablePageConfig = {
    autoReload: 10000,
    stateId: 'a957d3fe-4e06-11ea-b0a8-e3ba560c9a42',
    sorters: [
      {
        dir: 'asc',
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'RaidMgmt',
        get: {
          method: 'getList'
        }
      }
    },
    rowEnumFmt: '{{ devicefile }}',
    columns: [
      {
        name: gettext('Name'),
        prop: 'name',
        flexGrow: 1,
        hidden: true,
        sortable: true
      },
      {
        name: gettext('Device'),
        prop: 'devicefile',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('State'),
        prop: 'state',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Level'),
        prop: 'level',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          map: {
            stripe: { value: gettext('Stripe') },
            raid0: { value: gettext('Stripe') },
            mirror: { value: gettext('Mirror') },
            raid1: { value: gettext('Mirror') },
            linear: { value: gettext('Linear') },
            raid2: { value: gettext('RAID 2') },
            raid3: { value: gettext('RAID 3') },
            raid4: { value: gettext('RAID 4') },
            raid5: { value: gettext('RAID 5') },
            raid6: { value: gettext('RAID 6') },
            raid10: { value: gettext('RAID 10') }
          }
        }
      },
      {
        name: gettext('Capacity'),
        prop: 'size',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'binaryUnit'
      },
      {
        name: gettext('Devices'),
        prop: 'devices',
        flexGrow: 2,
        sortable: false,
        cellTemplateName: 'unsortedList'
      }
    ],
    actions: [
      {
        template: 'create',
        execute: {
          type: 'url',
          url: '/storage/md/create'
        }
      },
      {
        type: 'iconButton',
        icon: 'expand',
        tooltip: gettext('Grow'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          // Only RAID level 1/4/5/6 are able to grow.
          constraint: [
            {
              operator: 'in',
              arg0: { prop: 'level' },
              arg1: ['raid1', 'stripe', 'raid4', 'raid5', 'raid6']
            },
            {
              operator: 'in',
              arg0: { prop: 'state' },
              arg1: ['clean', 'active']
            }
          ]
        },
        execute: {
          type: 'url',
          url: '/storage/md/grow/{{ _selected[0].devicefile }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'remove',
        tooltip: gettext('Remove'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          // Only RAID level 1/2/3/4/5/6/10 are tolerant enough
          // to be able to remove slave/component devices without
          // loosing the whole array.
          constraint: [
            {
              operator: 'in',
              arg0: { prop: 'level' },
              arg1: ['raid1', 'mirror', 'raid2', 'raid3', 'raid4', 'raid5', 'raid6', 'raid10']
            },
            {
              operator: 'in',
              arg0: { prop: 'state' },
              arg1: ['clean', 'active']
            }
          ]
        },
        execute: {
          type: 'url',
          url: '/storage/md/remove/{{ _selected[0].devicefile }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'mdi:medical-bag',
        tooltip: gettext('Recover'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'url',
          url: '/storage/md/recover/{{ _selected[0].devicefile }}'
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
          url: '/storage/md/details/{{ _selected[0].devicefile }}'
        }
      },
      {
        template: 'delete',
        enabledConstraints: {
          constraint: [{ operator: 'falsy', arg0: { prop: '_used' } }]
        },
        execute: {
          type: 'request',
          request: {
            service: 'RaidMgmt',
            method: 'delete',
            params: {
              devicefile: '{{ devicefile }}'
            }
          }
        }
      }
    ]
  };
}
