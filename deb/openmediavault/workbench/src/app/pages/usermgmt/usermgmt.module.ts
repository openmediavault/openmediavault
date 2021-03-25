import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CoreModule } from '~/app/core/core.module';
import { GroupDatatablePageComponent } from '~/app/pages/usermgmt/groups/group-datatable-page.component';
import { GroupFormPageComponent } from '~/app/pages/usermgmt/groups/group-form-page.component';
import { GroupImportFormPageComponent } from '~/app/pages/usermgmt/groups/group-import-form-page.component';
import { GroupPrivilegesDatatablePageComponent } from '~/app/pages/usermgmt/groups/group-privileges-datatable-page.component';
import { UsermgmtRoutingModule } from '~/app/pages/usermgmt/usermgmt-routing.module';
import { UserDatatablePageComponent } from '~/app/pages/usermgmt/users/user-datatable-page.component';
import { UserFormPageComponent } from '~/app/pages/usermgmt/users/user-form-page.component';
import { UserImportFormPageComponent } from '~/app/pages/usermgmt/users/user-import-form-page.component';
import { UserPasswordFormPageComponent } from '~/app/pages/usermgmt/users/user-password-form-page.component';
import { UserPrivilegesDatatablePageComponent } from '~/app/pages/usermgmt/users/user-privileges-datatable-page.component';
import { UserProfileFormPageComponent } from '~/app/pages/usermgmt/users/user-profile-form-page.component';
import { UserSettingsFormPageComponent } from '~/app/pages/usermgmt/users/user-settings-form-page.component';

@NgModule({
  declarations: [
    UserDatatablePageComponent,
    UserFormPageComponent,
    UserPasswordFormPageComponent,
    UserSettingsFormPageComponent,
    UserImportFormPageComponent,
    GroupFormPageComponent,
    GroupImportFormPageComponent,
    GroupDatatablePageComponent,
    GroupPrivilegesDatatablePageComponent,
    UserPrivilegesDatatablePageComponent,
    UserProfileFormPageComponent
  ],
  imports: [CommonModule, CoreModule, UsermgmtRoutingModule]
})
export class UsermgmtModule {}
