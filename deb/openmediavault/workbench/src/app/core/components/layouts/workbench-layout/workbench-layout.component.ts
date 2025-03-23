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
import { BreakpointObserver, Breakpoints } from '@angular/cdk/layout';
import { Component, OnInit, ViewChild } from '@angular/core';
import { MatDrawerMode, MatSidenav } from '@angular/material/sidenav';
import { Event, NavigationEnd, Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { forkJoin, Subscription } from 'rxjs';
import { delay, filter, finalize, take, tap } from 'rxjs/operators';

import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';
import { LogConfigService } from '~/app/core/services/log-config.service';
import { MkfsConfigService } from '~/app/core/services/mkfs-config.service';
import { NavigationConfigService } from '~/app/core/services/navigation-config.service';
import { Unsubscribe } from '~/app/decorators';
import { translate } from '~/app/i18n.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { PrefersColorSchemeService } from '~/app/shared/services/prefers-color-scheme.service';
import { SystemInformationService } from '~/app/shared/services/system-information.service';
import { RunningTasks, TaskRunnerService } from '~/app/shared/services/task-runner.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

@Component({
  selector: 'omv-workbench-layout',
  templateUrl: './workbench-layout.component.html',
  styleUrls: ['./workbench-layout.component.scss']
})
export class WorkbenchLayoutComponent implements OnInit {
  @ViewChild('navigationSidenav', { static: false })
  private navigationSidenav: MatSidenav;

  @ViewChild('notificationSidenav', { static: false })
  private notificationSidenav: MatSidenav;

  @Unsubscribe()
  private subscriptions = new Subscription();

  public loading = true;
  public sideNavMode: MatDrawerMode;
  public sideNavOpened = false;
  public displayWelcomeMessage = false;

  private isSmallScreen: boolean;

  constructor(
    private authSessionService: AuthSessionService,
    private blockUiService: BlockUiService,
    private dashboardWidgetConfigService: DashboardWidgetConfigService,
    private breakpointObserver: BreakpointObserver,
    private navigationConfig: NavigationConfigService,
    private notificationService: NotificationService,
    private prefersColorSchemeService: PrefersColorSchemeService,
    private router: Router,
    private logConfigService: LogConfigService,
    private mkfsConfigService: MkfsConfigService,
    private systemInformationService: SystemInformationService,
    private taskRunnerService: TaskRunnerService,
    private userLocalStorageService: UserLocalStorageService
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
      breakpointObserver.observe([Breakpoints.XSmall, Breakpoints.Small]).subscribe(() => {
        this.updateState();
      })
    );
  }

  ngOnInit(): void {
    this.updateState();
  }

  onToggleNavigation() {
    this.navigationSidenav?.toggle();
  }

  onToggleNotification() {
    this.notificationSidenav?.toggle();
  }

  private updateState() {
    this.isSmallScreen =
      this.breakpointObserver.isMatched(Breakpoints.XSmall) ||
      this.breakpointObserver.isMatched(Breakpoints.Small);
    setTimeout(() => {
      this.sideNavOpened = !this.isSmallScreen;
      this.sideNavMode = this.isSmallScreen ? 'over' : 'side';
    });
  }

  private initLayout(): void {
    // Load the navigation, dashboard widgets and logging configuration.
    // Additionally, load the users local storage settings and apply them
    // to the browsers local storage.
    this.loading = true;
    this.blockUiService.start(translate(gettext('Loading ...')));
    forkJoin([
      this.navigationConfig.load(),
      this.dashboardWidgetConfigService.load(),
      this.logConfigService.load(),
      this.mkfsConfigService.load(),
      this.userLocalStorageService.load().pipe(
        tap(() => {
          // The theme settings may need to be synchronized and applied
          // after the local storage settings have been transferred from
          // the server.
          this.prefersColorSchemeService.sync();
        })
      )
    ])
      .pipe(
        // Delay a second, otherwise the display of the loading progress
        // bar looks like screen flickering.
        delay(1000),
        finalize(() => {
          this.loading = false;
          this.blockUiService.stop();
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
