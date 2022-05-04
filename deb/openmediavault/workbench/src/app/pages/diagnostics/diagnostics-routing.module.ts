/* eslint-disable max-len */
import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { PerformanceStatisticCpuRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-cpu-rrd-page.component';
import { PerformanceStatisticDuRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-du-rrd-page.component';
import { PerformanceStatisticLoadRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-load-rrd-page.component';
import { PerformanceStatisticMemoryRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-memory-rrd-page.component';
import { PerformanceStatisticNetworkRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-network-rrd-page.component';
import { PerformanceStatisticUptimeRrdPageComponent } from '~/app/pages/diagnostics/performance-statistics/performance-statistic-uptime-rrd-page.component';
import { ProcessesTextPageComponent } from '~/app/pages/diagnostics/processes/processes-text-page.component';
import { ReportTextPageComponent } from '~/app/pages/diagnostics/report/report-text-page.component';
import { ServiceFtpTextPageComponent } from '~/app/pages/diagnostics/services/service-ftp-text-page.component';
import { ServiceSmbTextPageComponent } from '~/app/pages/diagnostics/services/service-smb-text-page.component';
import { ServiceSshTextPageComponent } from '~/app/pages/diagnostics/services/service-ssh-text-page.component';
import { SystemInformationDatatablePageComponent } from '~/app/pages/diagnostics/system-information/system-information-datatable-page.component';
import { SystemLogsListPageComponent } from '~/app/pages/diagnostics/system-logs/system-logs-list-page.component';
import { SystemLogsRemoteFormPageComponent } from '~/app/pages/diagnostics/system-logs/system-logs-remote-form-page.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'system-information',
    component: SystemInformationDatatablePageComponent,
    data: { title: gettext('System Information') }
  },
  {
    path: 'processes',
    component: ProcessesTextPageComponent,
    data: { title: gettext('Processes') }
  },
  {
    path: 'report',
    component: ReportTextPageComponent,
    data: { title: gettext('Report') }
  },
  {
    path: 'system-logs',
    data: { title: gettext('System Logs') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'logs',
        component: SystemLogsListPageComponent,
        data: { title: gettext('Logs') }
      },
      {
        path: 'remote',
        component: SystemLogsRemoteFormPageComponent,
        data: { title: gettext('Remote') }
      }
    ]
  },
  {
    path: 'performance-statistics',
    data: { title: gettext('Performance Statistics') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'cpu',
        component: PerformanceStatisticCpuRrdPageComponent,
        data: { title: gettext('CPU') }
      },
      {
        path: 'disk-usage',
        component: PerformanceStatisticDuRrdPageComponent,
        data: { title: gettext('Disk Usage') }
      },
      {
        path: 'load',
        component: PerformanceStatisticLoadRrdPageComponent,
        data: { title: gettext('Load Average') }
      },
      {
        path: 'memory',
        component: PerformanceStatisticMemoryRrdPageComponent,
        data: { title: gettext('Memory Usage') }
      },
      {
        path: 'network-interfaces',
        component: PerformanceStatisticNetworkRrdPageComponent,
        data: { title: gettext('Network Interfaces') }
      },
      {
        path: 'uptime',
        component: PerformanceStatisticUptimeRrdPageComponent,
        data: { title: gettext('Uptime') }
      }
    ]
  },
  {
    path: 'services',
    data: { title: gettext('Services') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'ftp',
        component: ServiceFtpTextPageComponent,
        data: { title: gettext('FTP') }
      },
      {
        path: 'smb',
        component: ServiceSmbTextPageComponent,
        data: { title: gettext('SMB/CIFS') }
      },
      {
        path: 'ssh',
        component: ServiceSshTextPageComponent,
        data: { title: gettext('SSH') }
      }
    ]
  }
];

@NgModule({
  exports: [RouterModule],
  imports: [RouterModule.forChild(routes)],
  providers: [
    {
      provide: ROUTES,
      multi: true,
      useFactory: (routeConfigService: RouteConfigService): Routes => {
        routeConfigService.inject('diagnostics', routes);
        return routes;
      },
      deps: [RouteConfigService]
    }
  ]
})
export class DiagnosticsRoutingModule {}
