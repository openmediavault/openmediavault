/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
