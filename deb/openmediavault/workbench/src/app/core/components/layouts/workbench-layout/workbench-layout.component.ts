import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { MediaObserver } from '@angular/flex-layout';
import { MatSidenav } from '@angular/material/sidenav';
import { MatDrawerMode } from '@angular/material/sidenav/drawer';
import { Event, NavigationEnd, Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { forkJoin, Subscription } from 'rxjs';
import { delay, filter, finalize } from 'rxjs/operators';

import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';
import { LogConfigService } from '~/app/core/services/log-config.service';
import { NavigationConfigService } from '~/app/core/services/navigation-config.service';
import { translate } from '~/app/i18n.helper';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';

@Component({
  selector: 'omv-workbench-layout',
  templateUrl: './workbench-layout.component.html',
  styleUrls: ['./workbench-layout.component.scss']
})
export class WorkbenchLayoutComponent implements OnInit, OnDestroy {
  @BlockUI()
  blockUI: NgBlockUI;

  @ViewChild('navigationSidenav', { static: true })
  private navigationSidenav: MatSidenav;

  public loading = true;
  public sideNavMode: MatDrawerMode;
  public sideNavOpened = false;
  public pendingChanges = false;

  private isSmallScreen: boolean;
  private subscriptions = new Subscription();

  constructor(
    private dashboardWidgetConfigService: DashboardWidgetConfigService,
    private media: MediaObserver,
    private navigationConfig: NavigationConfigService,
    private router: Router,
    private systemInformationService: SystemInformationService,
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
    this.subscriptions.add(
      this.systemInformationService.systemInfo$.subscribe((res: SystemInformation) => {
        this.pendingChanges = _.get(res, 'configDirty', false) as boolean;
      })
    );
  }

  ngOnInit(): void {
    this.updateState();
  }

  ngOnDestroy(): void {
    this.subscriptions.unsubscribe();
  }

  private updateState() {
    this.isSmallScreen = this.media.isActive('xs') || this.media.isActive('sm');
    setTimeout(() => {
      this.sideNavOpened = !this.isSmallScreen;
      this.sideNavMode = this.isSmallScreen ? 'over' : 'side';
    });
  }
}
