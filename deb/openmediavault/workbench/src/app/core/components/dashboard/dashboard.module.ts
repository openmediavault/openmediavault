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
/* eslint-disable max-len */
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { RouterModule } from '@angular/router';
import { TranslocoModule } from '@ngneat/transloco';

import { DashboardWidgetComponent } from '~/app/core/components/dashboard/dashboard-widget/dashboard-widget.component';
import { DashboardWidgetChartComponent } from '~/app/core/components/dashboard/dashboard-widget-chart/dashboard-widget-chart.component';
import { DashboardWidgetDatatableComponent } from '~/app/core/components/dashboard/dashboard-widget-datatable/dashboard-widget-datatable.component';
import { DashboardWidgetFilesystemsStatusComponent } from '~/app/core/components/dashboard/dashboard-widget-filesystems-status/dashboard-widget-filesystems-status.component';
import { DashboardWidgetGridComponent } from '~/app/core/components/dashboard/dashboard-widget-grid/dashboard-widget-grid.component';
import { DashboardWidgetRrdComponent } from '~/app/core/components/dashboard/dashboard-widget-rrd/dashboard-widget-rrd.component';
import { DashboardWidgetSystemInformationComponent } from '~/app/core/components/dashboard/dashboard-widget-system-information/dashboard-widget-system-information.component';
import { DashboardWidgetTextComponent } from '~/app/core/components/dashboard/dashboard-widget-text/dashboard-widget-text.component';
import { DashboardWidgetValueComponent } from '~/app/core/components/dashboard/dashboard-widget-value/dashboard-widget-value.component';
import { MaterialModule } from '~/app/material.module';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [
    DashboardWidgetComponent,
    DashboardWidgetDatatableComponent,
    DashboardWidgetRrdComponent,
    DashboardWidgetSystemInformationComponent,
    DashboardWidgetChartComponent,
    DashboardWidgetFilesystemsStatusComponent,
    DashboardWidgetGridComponent,
    DashboardWidgetTextComponent,
    DashboardWidgetValueComponent
  ],
  exports: [
    DashboardWidgetDatatableComponent,
    DashboardWidgetRrdComponent,
    DashboardWidgetSystemInformationComponent,
    DashboardWidgetChartComponent,
    DashboardWidgetFilesystemsStatusComponent,
    DashboardWidgetGridComponent,
    DashboardWidgetTextComponent,
    DashboardWidgetValueComponent
  ],
  imports: [CommonModule, MaterialModule, RouterModule, SharedModule, TranslocoModule]
})
export class DashboardModule {}
