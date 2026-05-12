import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CoreModule } from '~/app/core/core.module';
import { NfsSettingsFormPageComponent } from '~/app/pages/services/nfs/nfs-settings-form-page.component';
import { NfsShareDatatablePageComponent } from '~/app/pages/services/nfs/nfs-share-datatable-page.component';
import { NfsShareFormPageComponent } from '~/app/pages/services/nfs/nfs-share-form-page.component';
import { RsyncModuleDatatablePageComponent } from '~/app/pages/services/rsync/rsync-module-datatable-page.component';
import { RsyncModuleFormPageComponent } from '~/app/pages/services/rsync/rsync-module-form-page.component';
import { RsyncModuleSettingsFormPageComponent } from '~/app/pages/services/rsync/rsync-module-settings-form-page.component';
import { RsyncTaskDatatablePageComponent } from '~/app/pages/services/rsync/rsync-task-datatable-page.component';
import { RsyncTaskFormPageComponent } from '~/app/pages/services/rsync/rsync-task-form-page.component';
import { ServicesRoutingModule } from '~/app/pages/services/services-routing.module';
import { SmbSettingsFormPageComponent } from '~/app/pages/services/smb/smb-settings-form-page.component';
import { SmbShareDatatablePageComponent } from '~/app/pages/services/smb/smb-share-datatable-page.component';
import { SmbShareFormPageComponent } from '~/app/pages/services/smb/smb-share-form-page.component';
import { SshFormPageComponent } from '~/app/pages/services/ssh/ssh-form-page.component';

@NgModule({
  declarations: [
    SshFormPageComponent,
    SmbSettingsFormPageComponent,
    SmbShareDatatablePageComponent,
    SmbShareFormPageComponent,
    NfsSettingsFormPageComponent,
    NfsShareDatatablePageComponent,
    NfsShareFormPageComponent,
    RsyncTaskDatatablePageComponent,
    RsyncTaskFormPageComponent,
    RsyncModuleSettingsFormPageComponent,
    RsyncModuleDatatablePageComponent,
    RsyncModuleFormPageComponent
  ],
  imports: [CommonModule, CoreModule, ServicesRoutingModule]
})
export class ServicesModule {}
