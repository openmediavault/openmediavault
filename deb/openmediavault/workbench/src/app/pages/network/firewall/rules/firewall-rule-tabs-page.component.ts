import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { TabsPageConfig } from '~/app/core/components/limn-ui/models/tabs-page-config.type';

@Component({
  template: '<omv-limn-tabs-page [config]="this.config"></omv-limn-tabs-page>'
})
export class FirewallRuleTabsPageComponent {
  public config: TabsPageConfig = {
    singleRoute: false,
    tabs: [
      {
        url: '/network/firewall/rules/inet',
        label: gettext('IPv4')
      },
      {
        url: '/network/firewall/rules/inet6',
        label: gettext('IPv6')
      }
    ]
  };
}
