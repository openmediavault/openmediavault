import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { RouterModule } from '@angular/router';
import { TranslocoModule } from '@ngneat/transloco';

import { ComponentsModule } from '~/app/core/components/components.module';
import { DashboardModule } from '~/app/core/components/dashboard/dashboard.module';
import { AboutPageComponent } from '~/app/core/pages/about-page/about-page.component';
import { BlankPageComponent } from '~/app/core/pages/blank-page/blank-page.component';
import { DashboardPageComponent } from '~/app/core/pages/dashboard-page/dashboard-page.component';
import { GuruMeditationPageComponent } from '~/app/core/pages/guru-meditation-page/guru-meditation-page.component';
import { LoginPageComponent } from '~/app/core/pages/login-page/login-page.component';
import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { ShutdownPageComponent } from '~/app/core/pages/shutdown-page/shutdown-page.component';
import { StandbyPageComponent } from '~/app/core/pages/standby-page/standby-page.component';
import { MaterialModule } from '~/app/material.module';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [
    GuruMeditationPageComponent,
    LoginPageComponent,
    NavigationPageComponent,
    ShutdownPageComponent,
    StandbyPageComponent,
    DashboardPageComponent,
    AboutPageComponent,
    BlankPageComponent
  ],
  exports: [
    GuruMeditationPageComponent,
    LoginPageComponent,
    NavigationPageComponent,
    ShutdownPageComponent,
    StandbyPageComponent,
    DashboardPageComponent,
    AboutPageComponent,
    BlankPageComponent
  ],
  imports: [
    CommonModule,
    SharedModule,
    ComponentsModule,
    MaterialModule,
    RouterModule,
    TranslocoModule,
    DashboardModule
  ]
})
export class PagesModule {}
