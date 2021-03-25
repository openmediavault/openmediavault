import { Component } from '@angular/core';

import { RrdPageConfig } from '~/app/core/components/limn-ui/models/rrd-page-config.type';

@Component({
  template: '<omv-limn-rrd-page [config]="this.config"></omv-limn-rrd-page>'
})
export class PerformanceStatisticUptimeRrdPageComponent {
  public config: RrdPageConfig = {
    graphs: [
      {
        name: 'uptime'
      }
    ]
  };
}
