import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { TranslocoModule } from '@ngneat/transloco';

import { CoreModule } from '~/app/core/core.module';
import { MaterialModule } from '~/app/material.module';
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
import { NotificationSelectionListPageComponent } from '~/app/pages/system/notification/notification-selection-list-page.component';
import { NotificationSettingsFormPageComponent } from '~/app/pages/system/notification/notification-settings-form-page.component';
import { PluginsDatatablePageComponent } from '~/app/pages/system/plugins/plugins-datatable-page.component';
import { PowermgmtSettingsFormPageComponent } from '~/app/pages/system/powermgmt/powermgmt-settings-form-page.component';
import { PowermgmtTaskDatatablePageComponent } from '~/app/pages/system/powermgmt/powermgmt-task-datatable-page.component';
import { PowermgmtTaskFormPageComponent } from '~/app/pages/system/powermgmt/powermgmt-task-form-page.component';
import { SystemRoutingModule } from '~/app/pages/system/system-routing.module';
import { UpdateDatatablePageComponent } from '~/app/pages/system/updates/update-datatable-page.component';
import { UpdateSettingsSoftwareFormPageComponent } from '~/app/pages/system/updates/update-settings-software-form-page.component';
import { UpdateSettingsUpdatesFormPageComponent } from '~/app/pages/system/updates/update-settings-updates-form-page.component';
import { WorkbenchFormPageComponent } from '~/app/pages/system/workbench/workbench-form-page.component';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [
    WorkbenchFormPageComponent,
    SshCertificateDatatablePageComponent,
    SslCertificateDatatablePageComponent,
    MonitoringFormPageComponent,
    DateTimeFormPageComponent,
    SslCertificateFormPageComponent,
    SslCertificateImportFormPageComponent,
    SslCertificateDetailsTextPageComponent,
    SshCertificateCreateFormPageComponent,
    SshCertificateEditFormPageComponent,
    SshCertificateImportFormPageComponent,
    NotificationSettingsFormPageComponent,
    NotificationSelectionListPageComponent,
    UpdateDatatablePageComponent,
    PluginsDatatablePageComponent,
    UpdateSettingsSoftwareFormPageComponent,
    UpdateSettingsUpdatesFormPageComponent,
    PowermgmtSettingsFormPageComponent,
    PowermgmtTaskDatatablePageComponent,
    PowermgmtTaskFormPageComponent,
    CronTaskDatatablePageComponent,
    CronTaskFormPageComponent
  ],
  imports: [
    CommonModule,
    CoreModule,
    MaterialModule,
    SharedModule,
    SystemRoutingModule,
    TranslocoModule
  ]
})
export class SystemModule {}
