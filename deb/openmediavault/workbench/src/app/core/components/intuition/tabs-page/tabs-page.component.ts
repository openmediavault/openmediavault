/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
import { Component, Inject } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import * as _ from 'lodash';

import { AbstractPageComponent } from '~/app/core/components/intuition/abstract-page-component';
import { TabsPageConfig } from '~/app/core/components/intuition/models/tabs-page-config.type';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';

/**
 * This component will render a page containing tabs.
 */
@Component({
  selector: 'omv-intuition-tabs-page',
  templateUrl: './tabs-page.component.html',
  styleUrls: ['./tabs-page.component.scss']
})
export class TabsPageComponent extends AbstractPageComponent<TabsPageConfig> {
  constructor(
    @Inject(ActivatedRoute) activatedRoute,
    @Inject(AuthSessionService) authSessionService: AuthSessionService,
    @Inject(Router) router: Router
  ) {
    super(activatedRoute, authSessionService, router);
  }

  protected override sanitizeConfig() {
    _.defaultsDeep(this.config, {
      singleRoute: true,
      tabs: []
    });
  }
}
