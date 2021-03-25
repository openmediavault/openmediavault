import { Component } from '@angular/core';

import { TextPageConfig } from '~/app/core/components/limn-ui/models/text-page-config.type';

@Component({
  template: '<omv-limn-text-page [config]="this.config"></omv-limn-text-page>'
})
export class MdDetailsTextPageComponent {
  public config: TextPageConfig = {
    request: {
      service: 'RaidMgmt',
      get: {
        method: 'getDetail',
        params: {
          devicefile: '{{ _routeParams.devicefile }}'
        }
      }
    },
    buttons: [
      {
        template: 'back',
        url: '/storage/md'
      }
    ]
  };
}
