import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CoreModule } from '~/app/core/core.module';
import { GroupDatatablePageComponent } from '~/app/pages/usermgmt/groups/group-datatable-page.component';
import { GroupFormPageComponent } from '~/app/pages/usermgmt/groups/group-form-page.component';
import { GroupImportFormPageComponent } from '~/app/pages/usermgmt/groups/group-import-form-page.component';
import { GroupSharedFolderPermissionsDatatablePageComponent } from '~/app/pages/usermgmt/groups/group-shared-folder-permissions-datatable-page.component';
import { UsermgmtRoutingModule } from '~/app/pages/usermgmt/usermgmt-routing.module';
import { UserDatatablePageComponent } from '~/app/pages/usermgmt/users/user-datatable-page.component';
import { UserFormPageComponent } from '~/app/pages/usermgmt/users/user-form-page.component';
import { UserImportFormPageComponent } from '~/app/pages/usermgmt/users/user-import-form-page.component';
import { UserSettingsFormPageComponent } from '~/app/pages/usermgmt/users/user-settings-form-page.component';
import { UserSharedFolderPermissionsDatatablePageComponent } from '~/app/pages/usermgmt/users/user-shared-folder-permissions-datatable-page.component';

@NgModule({
  declarations: [
    UserDatatablePageComponent,
    UserFormPageComponent,
    UserSettingsFormPageComponent,
    UserImportFormPageComponent,
    GroupFormPageComponent,
    GroupImportFormPageComponent,
    GroupDatatablePageComponent,
    GroupSharedFolderPermissionsDatatablePageComponent,
    UserSharedFolderPermissionsDatatablePageComponent
  ],
  imports: [CommonModule, CoreModule, UsermgmtRoutingModule]
})
export class UsermgmtModule {}
