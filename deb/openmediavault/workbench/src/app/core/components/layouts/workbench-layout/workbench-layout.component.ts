/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { MediaObserver } from '@angular/flex-layout';
import { MatDrawerMode, MatSidenav } from '@angular/material/sidenav';
import { Event, NavigationEnd, Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { forkJoin, Subscription } from 'rxjs';
import { delay, filter, finalize } from 'rxjs/operators';

import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';
import { LogConfigService } from '~/app/core/services/log-config.service';
import { NavigationConfigService } from '~/app/core/services/navigation-config.service';
import { translate } from '~/app/i18n.helper';

@Component({
  selector: 'omv-workbench-layout',
  templateUrl: './workbench-layout.component.html',
  styleUrls: ['./workbench-layout.component.scss']
})
export class WorkbenchLayoutComponent implements OnInit, OnDestroy {
  @BlockUI()
  blockUI: NgBlockUI;

  @ViewChild('navigationSidenav', { static: false })
  private navigationSidenav: MatSidenav;

  @ViewChild('notificationsSidenav', { static: false })
  private notificationsSidenav: MatSidenav;

  public loading = true;
  public sideNavMode: MatDrawerMode;
  public sideNavOpened = false;

  private isSmallScreen: boolean;
  private subscriptions = new Subscription();

  constructor(
    private dashboardWidgetConfigService: DashboardWidgetConfigService,
    private media: MediaObserver,
    private navigationConfig: NavigationConfigService,
    private router: Router,
    private logConfigService: LogConfigService
  ) {
    this.blockUI.start(translate(gettext('Loading ...')));
    // Load the navigation and dashboard widget configuration.
    forkJoin([
      this.navigationConfig.load(),
      this.dashboardWidgetConfigService.load(),
      this.logConfigService.load()
    ])
      .pipe(
        // Delay a second, otherwise the display of the loading progress
        // bar looks like screen flickering.
        delay(1000),
        finalize(() => {
          this.loading = false;
          this.blockUI.stop();
        })
      )
      .subscribe();
    // Do not subscribe on login page.
    this.subscriptions.add(
      this.router.events
        .pipe(filter((event: Event) => event instanceof NavigationEnd))
        .subscribe(() => {
          if (this.isSmallScreen) {
            // Is not available on login page.
            if (this.navigationSidenav) {
              this.navigationSidenav.close();
            }
          }
        })
    );
    this.subscriptions.add(
      media.asObservable().subscribe(() => {
        this.updateState();
      })
    );
  }

  ngOnInit(): void {
    this.updateState();
  }

  ngOnDestroy(): void {
    this.subscriptions.unsubscribe();
  }

  onToggleNavigationSidenav() {
    if (this.navigationSidenav) {
      this.navigationSidenav.toggle();
    }
  }

  onToggleNotificationsSidenav() {
    if (this.notificationsSidenav) {
      this.notificationsSidenav.toggle();
    }
  }

  private updateState() {
    this.isSmallScreen = this.media.isActive('xs') || this.media.isActive('sm');
    setTimeout(() => {
      this.sideNavOpened = !this.isSmallScreen;
      this.sideNavMode = this.isSmallScreen ? 'over' : 'side';
    });
  }
}
