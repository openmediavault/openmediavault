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
import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { MediaObserver } from '@angular/flex-layout';
import { MatDrawerMode, MatSidenav } from '@angular/material/sidenav';
import { Event, NavigationEnd, Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { forkJoin, Subscription } from 'rxjs';
import { delay, filter, finalize, take } from 'rxjs/operators';

import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';
import { LogConfigService } from '~/app/core/services/log-config.service';
import { NavigationConfigService } from '~/app/core/services/navigation-config.service';
import { translate } from '~/app/i18n.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { SystemInformationService } from '~/app/shared/services/system-information.service';
import { RunningTasks, TaskRunnerService } from '~/app/shared/services/task-runner.service';

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

  @ViewChild('notificationSidenav', { static: false })
  private notificationSidenav: MatSidenav;

  public loading = true;
  public sideNavMode: MatDrawerMode;
  public sideNavOpened = false;
  public displayWelcomeMessage = false;

  private isSmallScreen: boolean;
  private subscriptions = new Subscription();

  constructor(
    private authSessionService: AuthSessionService,
    private dashboardWidgetConfigService: DashboardWidgetConfigService,
    private media: MediaObserver,
    private navigationConfig: NavigationConfigService,
    private notificationService: NotificationService,
    private router: Router,
    private logConfigService: LogConfigService,
    private systemInformationService: SystemInformationService,
    private taskRunnerService: TaskRunnerService
  ) {
    this.initLayout();
    // Do not subscribe on login page.
    this.subscriptions.add(
      this.router.events
        .pipe(filter((event: Event) => event instanceof NavigationEnd))
        .subscribe(() => {
          if (this.isSmallScreen) {
            // Is not available on login page.
            this.navigationSidenav?.close();
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

  onToggleNavigation() {
    this.navigationSidenav?.toggle();
  }

  onToggleNotification() {
    this.notificationSidenav?.toggle();
  }

  private updateState() {
    this.isSmallScreen = this.media.isActive('xs') || this.media.isActive('sm');
    setTimeout(() => {
      this.sideNavOpened = !this.isSmallScreen;
      this.sideNavMode = this.isSmallScreen ? 'over' : 'side';
    });
  }

  private initLayout(): void {
    // Load the navigation, dashboard widgets and logging configuration.
    this.loading = true;
    this.blockUI.start(translate(gettext('Loading ...')));
    forkJoin([
      this.navigationConfig.load(),
      this.dashboardWidgetConfigService.loadWidgetConfig(),
      this.dashboardWidgetConfigService.loadUserWidgetConfig(),
      this.logConfigService.load()
    ])
      .pipe(
        // Delay a second, otherwise the display of the loading progress
        // bar looks like screen flickering.
        delay(1000),
        finalize(() => {
          this.loading = false;
          this.blockUI.stop();
          this.onAfterInitLayout();
        })
      )
      .subscribe();
  }

  private onAfterInitLayout(): void {
    if (this.authSessionService.hasAdminRole()) {
      this.taskRunnerService.enumerate().subscribe((tasks: RunningTasks) => {
        if (_.keys(tasks).length) {
          this.notificationService.show(
            NotificationType.info,
            gettext('A running background task was detected.'),
            gettext('Go to the notification sidebar to attach to it.')
          );
        }
      });
      this.systemInformationService.systemInfo$.pipe(take(1)).subscribe((sysInfo) => {
        this.displayWelcomeMessage = sysInfo.displayWelcomeMessage;
      });
    }
  }
}
