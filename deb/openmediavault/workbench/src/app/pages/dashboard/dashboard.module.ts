import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { TranslocoModule } from '@ngneat/transloco';

import { CoreModule } from '~/app/core/core.module';
import { MaterialModule } from '~/app/material.module';
import { DashboardRoutingModule } from '~/app/pages/dashboard/dashboard-routing.module';
import { DashboardSettingsPageComponent } from '~/app/pages/dashboard/dashboard-settings-page/dashboard-settings-page.component';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [DashboardSettingsPageComponent],
  imports: [
    CommonModule,
    CoreModule,
    DashboardRoutingModule,
    MaterialModule,
    SharedModule,
    TranslocoModule
  ]
})
export class DashboardModule {}
