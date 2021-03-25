import { Component } from '@angular/core';

import { TextPageConfig } from '~/app/core/components/limn-ui/models/text-page-config.type';

@Component({
  template: '<omv-limn-text-page [config]="this.config"></omv-limn-text-page>'
})
export class SslCertificateDetailsTextPageComponent {
  public config: TextPageConfig = {
    request: {
      service: 'CertificateMgmt',
      get: {
        method: 'getDetail',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      }
    },
    buttons: [
      {
        template: 'back',
        url: '/system/certificate/ssl'
      }
    ]
  };
}
