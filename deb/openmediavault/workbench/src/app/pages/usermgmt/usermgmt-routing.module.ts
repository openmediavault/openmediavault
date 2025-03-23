import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { GroupDatatablePageComponent } from '~/app/pages/usermgmt/groups/group-datatable-page.component';
import { GroupFormPageComponent } from '~/app/pages/usermgmt/groups/group-form-page.component';
import { GroupImportFormPageComponent } from '~/app/pages/usermgmt/groups/group-import-form-page.component';
import { GroupSharedFolderPermissionsDatatablePageComponent } from '~/app/pages/usermgmt/groups/group-shared-folder-permissions-datatable-page.component';
import { UserDatatablePageComponent } from '~/app/pages/usermgmt/users/user-datatable-page.component';
import { UserFormPageComponent } from '~/app/pages/usermgmt/users/user-form-page.component';
import { UserImportFormPageComponent } from '~/app/pages/usermgmt/users/user-import-form-page.component';
import { UserPasswordFormPageComponent } from '~/app/pages/usermgmt/users/user-password-form-page.component';
import { UserProfileFormPageComponent } from '~/app/pages/usermgmt/users/user-profile-form-page.component';
import { UserSettingsFormPageComponent } from '~/app/pages/usermgmt/users/user-settings-form-page.component';
import { UserSharedFolderPermissionsDatatablePageComponent } from '~/app/pages/usermgmt/users/user-shared-folder-permissions-datatable-page.component';
import { IsDirtyGuardService } from '~/app/shared/services/is-dirty-guard.service';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'change-password',
    component: UserPasswordFormPageComponent,
    canDeactivate: [IsDirtyGuardService],
    data: {
      title: gettext('Change Password'),
      editing: true,
      notificationTitle: gettext('Updated password.')
    }
  },
  {
    path: 'profile',
    component: UserProfileFormPageComponent,
    canDeactivate: [IsDirtyGuardService],
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
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created user "{{ name }}".')
        }
      },
      {
        path: 'import',
        component: UserImportFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: { title: gettext('Import'), notificationTitle: gettext('Imported users.') }
      },
      {
        path: 'edit/:name',
        component: UserFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated user "{{ name }}".')
        }
      },
      {
        path: 'permissions/:name',
        component: UserSharedFolderPermissionsDatatablePageComponent,
        data: {
          title: gettext('Permissions'),
          breadcrumb: {
            text: '{{ "Permissions" | translate }} @ {{ _routeParams.name }}'
          },
          notificationTitle: gettext('Updated permissions of user "{{ name }}".')
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
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Create'),
          editing: false,
          notificationTitle: gettext('Created group "{{ name }}".')
        }
      },
      {
        path: 'import',
        component: GroupImportFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: { title: gettext('Import'), notificationTitle: gettext('Imported groups.') }
      },
      {
        path: 'edit/:name',
        component: GroupFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated group "{{ name }}".')
        }
      },
      {
        path: 'permissions/:name',
        component: GroupSharedFolderPermissionsDatatablePageComponent,
        data: {
          title: gettext('Permissions'),
          breadcrumb: {
            text: '{{ "Permissions" | translate }} @ {{ _routeParams.name }}'
          },
          notificationTitle: gettext('Updated permissions of group "{{ name }}".')
        }
      }
    ]
  },
  {
    path: 'settings',
    component: UserSettingsFormPageComponent,
    canDeactivate: [IsDirtyGuardService],
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
