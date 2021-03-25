import { Component } from '@angular/core';

import { RrdPageConfig } from '~/app/core/components/limn-ui/models/rrd-page-config.type';

@Component({
  template: '<omv-limn-rrd-page [config]="this.config"></omv-limn-rrd-page>'
})
export class PerformanceStatisticDuRrdPageComponent {
  public config: RrdPageConfig = {
    graphs: [
      {
        name: 'df-{{ "root" if mountpoint == "/" else mountpoint | substr(1) | replace("/", "-") }}'
      }
    ],
    label:
      '{{ devicefile }}{{ " [" + label + "]" if label }}{{ " [" + "System" | translate + "]" if mountpoint == "/" }}',
    store: {
      proxy: {
        service: 'FileSystemMgmt',
        get: {
          method: 'enumerateMountedFilesystems',
          params: {
            includeroot: true
          }
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
