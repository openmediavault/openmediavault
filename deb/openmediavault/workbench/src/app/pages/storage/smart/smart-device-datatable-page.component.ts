import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';

@Component({
  template: '<omv-limn-datatable-page [config]="this.config"></omv-limn-datatable-page>'
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
            GOOD: { value: gettext('Good'), class: 'omv-chip-theme-success' },
            BAD_STATUS: { value: gettext('Unknown') },
            BAD_ATTRIBUTE_NOW: {
              value: gettext('Bad'),
              class: 'omv-chip-theme-error',
              tooltip: gettext('Device is being used outside design parameters.')
            },
            BAD_ATTRIBUTE_IN_THE_PAST: {
              value: gettext('Bad'),
              class: 'omv-chip-theme-error',
              tooltip: gettext('Device was used outside of design parameters in the past.')
            },
            BAD_SECTOR: {
              value: gettext('Bad'),
              class: 'omv-chip-theme-error',
              tooltip: gettext('Device has a few bad sectors.')
            },
            BAD_SECTOR_MANY: {
              value: gettext('Bad'),
              class: 'omv-chip-theme-error',
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
            '/storage/smart/devices/{{ "create" if _selected[0].uuid == newconfobjuuid else "edit" }}/{{ _selected[0].uuid }}/{{ _selected[0].devicefile }}'
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
          url: '/storage/smart/devices/details/{{ _selected[0].devicefile }}'
        }
      }
    ]
  };
}
