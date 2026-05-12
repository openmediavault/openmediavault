/* eslint-disable max-len */
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { RouterModule } from '@angular/router';
import { TranslocoModule } from '@ngneat/transloco';
import { NgScrollbarModule } from 'ngx-scrollbar';

import { ApplyConfigPanelComponent } from '~/app/core/components/apply-config-panel/apply-config-panel.component';
import { BreadcrumbComponent } from '~/app/core/components/breadcrumb/breadcrumb.component';
import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { GreenRainComponent } from '~/app/core/components/green-rain/green-rain.component';
import { GuruMeditationComponent } from '~/app/core/components/guru-meditation/guru-meditation.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { BlankLayoutComponent } from '~/app/core/components/layouts/blank-layout/blank-layout.component';
import { WorkbenchLayoutComponent } from '~/app/core/components/layouts/workbench-layout/workbench-layout.component';
import { NavigationBarComponent } from '~/app/core/components/navigation-bar/navigation-bar.component';
import { NavigationBarListItemComponent } from '~/app/core/components/navigation-bar/navigation-bar-list-item/navigation-bar-list-item.component';
import { NotificationBarComponent } from '~/app/core/components/notification-bar/notification-bar.component';
import { TopBarComponent } from '~/app/core/components/top-bar/top-bar.component';
import { WelcomePanelComponent } from '~/app/core/components/welcome-panel/welcome-panel.component';
import { MaterialModule } from '~/app/material.module';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [
    NavigationBarComponent,
    NavigationBarListItemComponent,
    TopBarComponent,
    GuruMeditationComponent,
    BreadcrumbComponent,
    NotificationBarComponent,
    BlankLayoutComponent,
    WorkbenchLayoutComponent,
    ApplyConfigPanelComponent,
    GreenRainComponent,
    WelcomePanelComponent
  ],
  exports: [
    BlankLayoutComponent,
    WorkbenchLayoutComponent,
    GuruMeditationComponent,
    IntuitionModule,
    NotificationBarComponent,
    DashboardModule,
    GreenRainComponent
  ],
  imports: [
    CommonModule,
    SharedModule,
    RouterModule,
    MaterialModule,
    IntuitionModule,
    TranslocoModule,
    NgScrollbarModule,
    DashboardModule
  ]
})
export class ComponentsModule {}
