import { Component } from '@angular/core';

import { RrdPageConfig } from '~/app/core/components/limn-ui/models/rrd-page-config.type';

@Component({
  template: '<omv-limn-rrd-page [config]="this.config"></omv-limn-rrd-page>'
})
export class PerformanceStatisticNetworkRrdPageComponent {
  public config: RrdPageConfig = {
    graphs: [
      {
        name: 'interface-{{ devicename }}'
      }
    ],
    label: '{{ devicename }}',
    store: {
      proxy: {
        service: 'Network',
        get: {
          method: 'enumerateConfiguredDevices'
        }
      },
      sorters: [
        {
          prop: 'devicefile',
          dir: 'asc'
        }
      ]
    }
  };
}
