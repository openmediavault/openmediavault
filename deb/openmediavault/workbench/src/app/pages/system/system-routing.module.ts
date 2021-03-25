import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { SshCertificateCreateFormPageComponent } from '~/app/pages/system/certificates/ssh/ssh-certificate-create-form-page.component';
import { SshCertificateDatatablePageComponent } from '~/app/pages/system/certificates/ssh/ssh-certificate-datatable-page.component';
import { SshCertificateEditFormPageComponent } from '~/app/pages/system/certificates/ssh/ssh-certificate-edit-form-page.component';
import { SshCertificateImportFormPageComponent } from '~/app/pages/system/certificates/ssh/ssh-certificate-import-form-page.component';
import { SslCertificateDatatablePageComponent } from '~/app/pages/system/certificates/ssl/ssl-certificate-datatable-page.component';
import { SslCertificateDetailsTextPageComponent } from '~/app/pages/system/certificates/ssl/ssl-certificate-details-text-page.component';
import { SslCertificateFormPageComponent } from '~/app/pages/system/certificates/ssl/ssl-certificate-form-page.component';
import { SslCertificateImportFormPageComponent } from '~/app/pages/system/certificates/ssl/ssl-certificate-import-form-page.component';
import { CronTaskDatatablePageComponent } from '~/app/pages/system/cron/cron-task-datatable-page.component';
import { CronTaskFormPageComponent } from '~/app/pages/system/cron/cron-task-form-page.component';
import { DateTimeFormPageComponent } from '~/app/pages/system/date-time/date-time-form-page.component';
import { MonitoringFormPageComponent } from '~/app/pages/system/monitoring/monitoring-form-page.component';
import { NotificationDatatablePageComponent } from '~/app/pages/system/notification/notification-datatable-page.component';
import { NotificationSettingsFormPageComponent } from '~/app/pages/system/notification/notification-settings-form-page.component';
import { PluginsDatatablePageComponent } from '~/app/pages/system/plugins/plugins-datatable-page.component';
import { PowermgmtSettingsFormPageComponent } from '~/app/pages/system/powermgmt/powermgmt-settings-form-page.component';
import { PowermgmtTaskDatatablePageComponent } from '~/app/pages/system/powermgmt/powermgmt-task-datatable-page.component';
import { PowermgmtTaskFormPageComponent } from '~/app/pages/system/powermgmt/powermgmt-task-form-page.component';
import { UpdateDatatablePageComponent } from '~/app/pages/system/updates/update-datatable-page.component';
import { UpdateSettingsFormPageComponent } from '~/app/pages/system/updates/update-settings-form-page.component';
import { WorkbenchFormPageComponent } from '~/app/pages/system/workbench/workbench-form-page.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'workbench',
    component: WorkbenchFormPageComponent,
    data: {
      title: gettext('Workbench'),
      notificationTitle: gettext('Updated workbench settings.'),
      editing: true
    }
  },
  {
    path: 'certificate',
    data: { title: gettext('Certificates') },
    children: [
      {
        path: '',
        component: NavigationPageComponent
      },
      {
        path: 'ssl',
        data: { title: gettext('SSL') },
        children: [
          { path: '', component: SslCertificateDatatablePageComponent },
          {
            path: 'create',
            component: SslCertificateFormPageComponent,
            data: {
              title: gettext('Create'),
              notificationTitle: gettext('Created SSL certificate.')
            }
          },
          {
            path: 'detail/:uuid',
            component: SslCertificateDetailsTextPageComponent,
            data: { title: gettext('Details') }
          },
          {
            path: 'import',
            component: SslCertificateImportFormPageComponent,
            data: {
              title: gettext('Import'),
              notificationTitle: gettext('Imported SSL certificate.')
            }
          }
        ]
      },
      {
        path: 'ssh',
        data: { title: gettext('SSH') },
        children: [
          { path: '', component: SshCertificateDatatablePageComponent },
          {
            path: 'create',
            component: SshCertificateCreateFormPageComponent,
            data: {
              title: gettext('Create'),
              notificationTitle: gettext('Created SSH certificate.')
            }
          },
          {
            path: 'edit/:uuid',
            component: SshCertificateEditFormPageComponent,
            data: {
              title: gettext('Edit'),
              notificationTitle: gettext('Updated SSH certificate.'),
              editing: true
            }
          },
          {
            path: 'import',
            component: SshCertificateImportFormPageComponent,
            data: {
              title: gettext('Import'),
              notificationTitle: gettext('Imported SSH certificate.')
            }
          }
        ]
      }
    ]
  },
  {
    path: 'monitoring',
    component: MonitoringFormPageComponent,
    data: {
      title: gettext('Monitoring'),
      notificationTitle: gettext('Updated monitoring settings.'),
      editing: true
    }
  },
  {
    path: 'notification',
    data: { title: gettext('Notification') },
    children: [
      {
        path: '',
        component: NavigationPageComponent
      },
      {
        path: 'settings',
        component: NotificationSettingsFormPageComponent,
        data: {
          title: gettext('Settings'),
          notificationTitle: gettext('Updated notification settings.'),
          editing: true
        }
      },
      {
        path: 'notifications',
        component: NotificationDatatablePageComponent,
        data: {
          title: gettext('Notifications'),
          notificationTitle: gettext('Updated notification settings.')
        }
      }
    ]
  },
  {
    path: 'date-time',
    component: DateTimeFormPageComponent,
    data: {
      title: gettext('Date & Time'),
      notificationTitle: gettext('Updated date & time settings.'),
      editing: true
    }
  },
  {
    path: 'powermgmt',
    data: { title: gettext('Power Management') },
    children: [
      {
        path: '',
        component: NavigationPageComponent
      },
      {
        path: 'settings',
        component: PowermgmtSettingsFormPageComponent,
        data: {
          title: gettext('Settings'),
          notificationTitle: gettext('Updated power management settings.'),
          editing: true
        }
      },
      {
        path: 'tasks',
        data: { title: gettext('Scheduled Tasks') },
        children: [
          { path: '', component: PowermgmtTaskDatatablePageComponent },
          {
            path: 'create',
            component: PowermgmtTaskFormPageComponent,
            data: {
              title: gettext('Create'),
              notificationTitle: gettext('Created scheduled task.'),
              editing: false
            }
          },
          {
            path: 'edit/:uuid',
            component: PowermgmtTaskFormPageComponent,
            data: {
              title: gettext('Edit'),
              notificationTitle: gettext('Updated scheduled task.'),
              editing: true
            }
          }
        ]
      }
    ]
  },
  {
    path: 'cron',
    data: { title: gettext('Scheduled Tasks') },
    children: [
      { path: '', component: CronTaskDatatablePageComponent },
      {
        path: 'create',
        component: CronTaskFormPageComponent,
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created scheduled task.')
        }
      },
      {
        path: 'edit/:uuid',
        component: CronTaskFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated scheduled task.')
        }
      }
    ]
  },
  {
    path: 'updatemgmt',
    data: { title: gettext('Update Management') },
    children: [
      {
        path: '',
        component: NavigationPageComponent
      },
      {
        path: 'updates',
        component: UpdateDatatablePageComponent,
        data: { title: gettext('Updates') }
      },
      {
        path: 'settings',
        component: UpdateSettingsFormPageComponent,
        data: {
          title: gettext('Settings'),
          notificationTitle: gettext('Updated update management settings.'),
          editing: true
        }
      }
    ]
  },
  {
    path: 'plugins',
    component: PluginsDatatablePageComponent,
    data: { title: gettext('Plugins') }
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
        routeConfigService.inject('system', routes);
        return routes;
      },
      deps: [RouteConfigService]
    }
  ]
})
export class SystemRoutingModule {}
