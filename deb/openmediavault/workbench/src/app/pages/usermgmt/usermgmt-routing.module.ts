import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { GroupDatatablePageComponent } from '~/app/pages/usermgmt/groups/group-datatable-page.component';
import { GroupFormPageComponent } from '~/app/pages/usermgmt/groups/group-form-page.component';
import { GroupImportFormPageComponent } from '~/app/pages/usermgmt/groups/group-import-form-page.component';
import { GroupPrivilegesDatatablePageComponent } from '~/app/pages/usermgmt/groups/group-privileges-datatable-page.component';
import { UserDatatablePageComponent } from '~/app/pages/usermgmt/users/user-datatable-page.component';
import { UserFormPageComponent } from '~/app/pages/usermgmt/users/user-form-page.component';
import { UserImportFormPageComponent } from '~/app/pages/usermgmt/users/user-import-form-page.component';
import { UserPasswordFormPageComponent } from '~/app/pages/usermgmt/users/user-password-form-page.component';
import { UserPrivilegesDatatablePageComponent } from '~/app/pages/usermgmt/users/user-privileges-datatable-page.component';
import { UserProfileFormPageComponent } from '~/app/pages/usermgmt/users/user-profile-form-page.component';
import { UserSettingsFormPageComponent } from '~/app/pages/usermgmt/users/user-settings-form-page.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'change-password',
    component: UserPasswordFormPageComponent,
    data: {
      title: gettext('Change Password'),
      editing: true,
      notificationTitle: gettext('Updated password.')
    }
  },
  {
    path: 'profile',
    component: UserProfileFormPageComponent,
    data: {
      title: gettext('Profile'),
      editing: true,
      notificationTitle: gettext('Updated user profile.')
    }
  },
  {
    path: 'users',
    data: { title: gettext('Users') },
    children: [
      { path: '', component: UserDatatablePageComponent },
      {
        path: 'create',
        component: UserFormPageComponent,
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created user "{{ name }}".')
        }
      },
      {
        path: 'import',
        component: UserImportFormPageComponent,
        data: { title: gettext('Import'), notificationTitle: gettext('Imported users.') }
      },
      {
        path: 'edit/:name',
        component: UserFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated user "{{ name }}".')
        }
      },
      {
        path: 'privileges/:name',
        component: UserPrivilegesDatatablePageComponent,
        data: {
          title: gettext('Privileges'),
          notificationTitle: gettext('Updated privileges of user "{{ name }}".')
        }
      }
    ]
  },
  {
    path: 'groups',
    data: { title: gettext('Groups') },
    children: [
      { path: '', component: GroupDatatablePageComponent },
      {
        path: 'create',
        component: GroupFormPageComponent,
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created group "{{ name }}".')
        }
      },
      {
        path: 'import',
        component: GroupImportFormPageComponent,
        data: { title: gettext('Import'), notificationTitle: gettext('Imported groups.') }
      },
      {
        path: 'edit/:name',
        component: GroupFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated group "{{ name }}".')
        }
      },
      {
        path: 'privileges/:name',
        component: GroupPrivilegesDatatablePageComponent,
        data: {
          title: gettext('Privileges'),
          notificationTitle: gettext('Updated privileges of group "{{ name }}".')
        }
      }
    ]
  },
  {
    path: 'settings',
    component: UserSettingsFormPageComponent,
    data: {
      title: gettext('Settings'),
      editing: true,
      notificationTitle: gettext('Updated settings.')
    }
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
        routeConfigService.inject('usermgmt', routes);
        return routes;
      },
      deps: [RouteConfigService]
    }
  ]
})
export class UsermgmtRoutingModule {}
