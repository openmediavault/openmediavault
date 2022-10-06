/* eslint-disable max-len */
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { RouterModule } from '@angular/router';
import { TranslateModule } from '@ngx-translate/core';
import { BlockUIModule } from 'ng-block-ui';
import { NgScrollbarModule } from 'ngx-scrollbar';

import { ApplyConfigComponent } from '~/app/core/components/apply-config/apply-config.component';
import { BreadcrumbComponent } from '~/app/core/components/breadcrumb/breadcrumb.component';
import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { GreenRainComponent } from '~/app/core/components/green-rain/green-rain.component';
import { GuruMeditationComponent } from '~/app/core/components/guru-meditation/guru-meditation.component';
import { IntuitionModule } from '~/app/core/components/intuition/intuition.module';
import { BlankLayoutComponent } from '~/app/core/components/layouts/blank-layout/blank-layout.component';
import { WorkbenchLayoutComponent } from '~/app/core/components/layouts/workbench-layout/workbench-layout.component';
import { NavigationBarComponent } from '~/app/core/components/navigation-bar/navigation-bar.component';
import { NavigationBarListItemComponent } from '~/app/core/components/navigation-bar/navigation-bar-list-item/navigation-bar-list-item.component';
import { NotificationsComponent } from '~/app/core/components/notifications/notifications.component';
import { TopBarComponent } from '~/app/core/components/top-bar/top-bar.component';
import { MaterialModule } from '~/app/material.module';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [
    NavigationBarComponent,
    NavigationBarListItemComponent,
    TopBarComponent,
    GuruMeditationComponent,
    BreadcrumbComponent,
    NotificationsComponent,
    BlankLayoutComponent,
    WorkbenchLayoutComponent,
    ApplyConfigComponent,
    GreenRainComponent
  ],
  exports: [
    BlankLayoutComponent,
    WorkbenchLayoutComponent,
    GuruMeditationComponent,
    IntuitionModule,
    NotificationsComponent,
    DashboardModule,
    GreenRainComponent
  ],
  imports: [
    BlockUIModule.forRoot(),
    CommonModule,
    SharedModule,
    RouterModule,
    MaterialModule,
    FlexLayoutModule,
    IntuitionModule,
    TranslateModule.forChild(),
    NgScrollbarModule,
    DashboardModule
  ]
})
export class ComponentsModule {}
