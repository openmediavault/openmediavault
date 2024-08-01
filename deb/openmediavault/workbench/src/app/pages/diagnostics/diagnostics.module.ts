/* eslint-disable max-len */
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CoreModule } from '~/app/core/core.module';
import { DiagnosticsRoutingModule } from '~/app/pages/diagnostics/diagnostics-routing.module';
import { PerformanceStatisticCpuRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-cpu-rrd-page.component';
import { PerformanceStatisticDfRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-df-rrd-page.component';
import { PerformanceStatisticLoadRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-load-rrd-page.component';
import { PerformanceStatisticMemoryRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-memory-rrd-page.component';
import { PerformanceStatisticNetworkRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-network-rrd-page.component';
import { PerformanceStatisticUptimeRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-uptime-rrd-page.component';
import { ProcessesTextPageComponent } from '~/app/pages/diagnostics/processes/processes-text-page.component';
import { ReportTextPageComponent } from '~/app/pages/diagnostics/report/report-text-page.component';
import { ServiceNfsTextPageComponent } from '~/app/pages/diagnostics/services/service-nfs-text-page.component';
import { ServiceSmbTextPageComponent } from '~/app/pages/diagnostics/services/service-smb-text-page.component';
import { ServiceSshTextPageComponent } from '~/app/pages/diagnostics/services/service-ssh-text-page.component';
import { SystemInformationDatatablePageComponent } from '~/app/pages/diagnostics/system-information/system-information-datatable-page.component';
import { SystemLogsListPageComponent } from '~/app/pages/diagnostics/system-logs/system-logs-list-page.component';
import { SystemLogsRemoteFormPageComponent } from '~/app/pages/diagnostics/system-logs/system-logs-remote-form-page.component';

@NgModule({
  declarations: [
    SystemInformationDatatablePageComponent,
    SystemLogsListPageComponent,
    SystemLogsRemoteFormPageComponent,
    ProcessesTextPageComponent,
    ReportTextPageComponent,
    PerformanceStatisticCpuRrdPageComponent,
    PerformanceStatisticDfRrdPageComponent,
    PerformanceStatisticLoadRrdPageComponent,
    PerformanceStatisticMemoryRrdPageComponent,
    PerformanceStatisticNetworkRrdPageComponent,
    PerformanceStatisticUptimeRrdPageComponent,
    ServiceNfsTextPageComponent,
    ServiceSmbTextPageComponent,
    ServiceSshTextPageComponent
  ],
  imports: [CommonModule, CoreModule, DiagnosticsRoutingModule]
})
export class DiagnosticsModule {}
