/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2023 Volker Theile
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
import { Component, OnInit } from '@angular/core';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import { Observable } from 'rxjs';
import { map, withLatestFrom } from 'rxjs/operators';

import { DashboardUserConfig } from '~/app/core/components/dashboard/models/dashboard-user-config.model';
import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';

@Component({
  selector: 'omv-dashboard-page',
  templateUrl: './dashboard-page.component.html',
  styleUrls: ['./dashboard-page.component.scss']
})
export class DashboardPageComponent implements OnInit {
  public widgets$: Observable<Array<DashboardWidgetConfig>>;

  public notConfiguredMessage: string = gettext(
    "The dashboard has not yet been configured. To personalize it, please go to the <a href='#/dashboard/settings'>settings page</a>."
  );

  constructor(private dashboardWidgetConfigService: DashboardWidgetConfigService) {}

  ngOnInit(): void {
    this.widgets$ = this.dashboardWidgetConfigService.getEnabled().pipe(
      withLatestFrom(this.dashboardWidgetConfigService.configs$),
      map(([enabledWidgets, widgets]: [DashboardUserConfig, Array<DashboardWidgetConfig>]) => {
        return widgets.filter((m) => enabledWidgets.widgets.map((w) => w.id).includes(m.id));
      })
    );
  }
}
