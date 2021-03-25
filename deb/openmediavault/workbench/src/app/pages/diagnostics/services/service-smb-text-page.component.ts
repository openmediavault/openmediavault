import { Component } from '@angular/core';

import { TextPageConfig } from '~/app/core/components/limn-ui/models/text-page-config.type';

@Component({
  template: '<omv-limn-text-page [config]="this.config"></omv-limn-text-page>'
})
export class ServiceSmbTextPageComponent {
  public config: TextPageConfig = {
    hasReloadButton: true,
    request: {
      service: 'SMB',
      get: {
        method: 'getStats'
      }
    }
  };
}
