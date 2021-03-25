import { Component, Inject } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import * as _ from 'lodash';

import { AbstractPageComponent } from '~/app/core/components/limn-ui/abstract-page-component';
import { TabsPageConfig } from '~/app/core/components/limn-ui/models/tabs-page-config.type';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';

/**
 * This component will render a page containing tabs.
 */
@Component({
  selector: 'omv-limn-tabs-page',
  templateUrl: './tabs-page.component.html',
  styleUrls: ['./tabs-page.component.scss']
})
export class TabsPageComponent extends AbstractPageComponent<TabsPageConfig> {
  constructor(
    @Inject(ActivatedRoute) activatedRoute,
    @Inject(AuthSessionService) authSessionService: AuthSessionService
  ) {
    super(activatedRoute, authSessionService);
  }

  protected sanitizeConfig() {
    _.defaultsDeep(this.config, {
      singleRoute: true,
      tabs: []
    });
  }
}
