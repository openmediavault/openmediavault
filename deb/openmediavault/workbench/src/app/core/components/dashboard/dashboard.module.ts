/* eslint-disable max-len */
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { RouterModule } from '@angular/router';
import { TranslateModule } from '@ngx-translate/core';

import { WidgetChartComponent } from '~/app/core/components/dashboard/widgets/widget-chart/widget-chart.component';
import { WidgetDatatableComponent } from '~/app/core/components/dashboard/widgets/widget-datatable/widget-datatable.component';
import { WidgetFilesystemsStatusComponent } from '~/app/core/components/dashboard/widgets/widget-filesystems-status/widget-filesystems-status.component';
import { WidgetRrdComponent } from '~/app/core/components/dashboard/widgets/widget-rrd/widget-rrd.component';
import { WidgetServicesStatusComponent } from '~/app/core/components/dashboard/widgets/widget-services-status/widget-services-status.component';
import { WidgetSystemInformationComponent } from '~/app/core/components/dashboard/widgets/widget-system-information/widget-system-information.component';
import { MaterialModule } from '~/app/material.module';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [
    WidgetDatatableComponent,
    WidgetRrdComponent,
    WidgetSystemInformationComponent,
    WidgetChartComponent,
    WidgetServicesStatusComponent,
    WidgetFilesystemsStatusComponent
  ],
  exports: [
    WidgetDatatableComponent,
    WidgetRrdComponent,
    WidgetSystemInformationComponent,
    WidgetChartComponent,
    WidgetServicesStatusComponent,
    WidgetFilesystemsStatusComponent
  ],
  imports: [
    CommonModule,
    FlexLayoutModule,
    MaterialModule,
    RouterModule,
    SharedModule,
    TranslateModule.forChild()
  ]
})
export class DashboardModule {}
