/* eslint-disable max-len */
import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { DiskDatatablePageComponent } from '~/app/pages/storage/disks/disk-datatable-page.component';
import { DiskFormPageComponent } from '~/app/pages/storage/disks/disk-form-page.component';
import { FilesystemCreateFormPageComponent } from '~/app/pages/storage/filesystems/filesystem-create-form-page.component';
import { FilesystemDatatablePageComponent } from '~/app/pages/storage/filesystems/filesystem-datatable-page.component';
import { FilesystemEditFormPageComponent } from '~/app/pages/storage/filesystems/filesystem-edit-form-page.component';
import { FilesystemMountFormPageComponent } from '~/app/pages/storage/filesystems/filesystem-mount-form-page.component';
import { FilesystemQuotaDatatablePageComponent } from '~/app/pages/storage/filesystems/filesystem-quota-datatable-page.component';
import { FilesystemQuotaFormPageComponent } from '~/app/pages/storage/filesystems/filesystem-quota-form-page.component';
import { MdDatatablePageComponent } from '~/app/pages/storage/md/md-datatable-page.component';
import { MdDetailsTextPageComponent } from '~/app/pages/storage/md/md-details-text-page.component';
import { MdFormPageComponent } from '~/app/pages/storage/md/md-form-page.component';
import { MdGrowFormPageComponent } from '~/app/pages/storage/md/md-grow-form-page.component';
import { MdRecoverFormPageComponent } from '~/app/pages/storage/md/md-recover-form-page.component';
import { MdRemoveFormPageComponent } from '~/app/pages/storage/md/md-remove-form-page.component';
import { SharedFolderAclFormPageComponent } from '~/app/pages/storage/shared-folders/shared-folder-acl-form-page.component';
import { SharedFolderDatatablePageComponent } from '~/app/pages/storage/shared-folders/shared-folder-datatable-page.component';
import { SharedFolderFormPageComponent } from '~/app/pages/storage/shared-folders/shared-folder-form-page.component';
import { SharedFolderPrivilegesDatatablePageComponent } from '~/app/pages/storage/shared-folders/shared-folder-privileges-datatable-page.component';
import { SmartDeviceDatatablePageComponent } from '~/app/pages/storage/smart/smart-device-datatable-page.component';
import { SmartDeviceDetailsTabsPageComponent } from '~/app/pages/storage/smart/smart-device-details-tabs-page.component';
import { SmartDeviceFormPageComponent } from '~/app/pages/storage/smart/smart-device-form-page.component';
import { SmartSettingsFormPageComponent } from '~/app/pages/storage/smart/smart-settings-form-page.component';
import { SmartTaskDatatablePageComponent } from '~/app/pages/storage/smart/smart-task-datatable-page.component';
import { SmartTaskFormPageComponent } from '~/app/pages/storage/smart/smart-task-form-page.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationPageComponent
  },
  {
    path: 'disks',
    data: { title: gettext('Disks') },
    children: [
      { path: '', component: DiskDatatablePageComponent },
      {
        path: 'hdparm/create/:devicefile',
        component: DiskFormPageComponent,
        data: { title: gettext('Edit'), editing: false }
      },
      {
        path: 'hdparm/edit/:uuid',
        component: DiskFormPageComponent,
        data: { title: gettext('Edit'), editing: true }
      }
    ]
  },
  {
    path: 'smart',
    data: { title: gettext('S.M.A.R.T.') },
    children: [
      { path: '', component: NavigationPageComponent },
      {
        path: 'settings',
        component: SmartSettingsFormPageComponent,
        data: { title: gettext('Settings'), editing: true }
      },
      {
        path: 'devices',
        data: { title: gettext('Devices') },
        children: [
          { path: '', component: SmartDeviceDatatablePageComponent },
          {
            path: 'details/:devicefile',
            component: SmartDeviceDetailsTabsPageComponent,
            data: { title: gettext('Details'), editing: true }
          },
          {
            path: 'create/:uuid/:devicefile',
            component: SmartDeviceFormPageComponent,
            data: { title: gettext('Edit'), editing: false }
          },
          {
            path: 'edit/:uuid/:devicefile',
            component: SmartDeviceFormPageComponent,
            data: { title: gettext('Edit'), editing: true }
          }
        ]
      },
      {
        path: 'tasks',
        data: { title: gettext('Scheduled Tasks') },
        children: [
          { path: '', component: SmartTaskDatatablePageComponent },
          {
            path: 'create',
            component: SmartTaskFormPageComponent,
            data: { title: gettext('Create'), editing: false }
          },
          {
            path: 'edit/:uuid',
            component: SmartTaskFormPageComponent,
            data: { title: gettext('Edit'), editing: true }
          }
        ]
      }
    ]
  },
  {
    path: 'md',
    data: { title: gettext('RAID Management') },
    children: [
      { path: '', component: MdDatatablePageComponent },
      {
        path: 'create',
        component: MdFormPageComponent,
        data: { title: gettext('Create'), editing: false }
      },
      {
        path: 'grow/:devicefile',
        component: MdGrowFormPageComponent,
        data: { title: gettext('Grow'), editing: false }
      },
      {
        path: 'remove/:devicefile',
        component: MdRemoveFormPageComponent,
        data: { title: gettext('Remove'), editing: true }
      },
      {
        path: 'recover/:devicefile',
        component: MdRecoverFormPageComponent,
        data: { title: gettext('Recover'), editing: false }
      },
      {
        path: 'details/:devicefile',
        component: MdDetailsTextPageComponent,
        data: { title: gettext('Details') }
      }
    ]
  },
  {
    path: 'shared-folders',
    data: { title: gettext('Shared Folders') },
    children: [
      { path: '', component: SharedFolderDatatablePageComponent },
      {
        path: 'create',
        component: SharedFolderFormPageComponent,
        data: { title: gettext('Create'), editing: false }
      },
      {
        path: 'edit/:uuid',
        component: SharedFolderFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated shared folder.')
        }
      },
      {
        path: 'privileges/:uuid',
        component: SharedFolderPrivilegesDatatablePageComponent,
        data: {
          title: gettext('Privileges'),
          notificationTitle: gettext('Updated privileges of shared folder.')
        }
      },
      {
        path: 'acl/:uuid',
        component: SharedFolderAclFormPageComponent,
        data: {
          title: gettext('ACL'),
          notificationTitle: gettext('Updated access control list of shared folder.')
        }
      }
    ]
  },
  {
    path: 'filesystems',
    data: { title: gettext('File Systems') },
    children: [
      { path: '', component: FilesystemDatatablePageComponent },
      {
        path: 'create',
        component: FilesystemCreateFormPageComponent,
        data: { title: gettext('Create'), editing: false }
      },
      {
        path: 'mount',
        component: FilesystemMountFormPageComponent,
        data: { title: gettext('Mount'), editing: false }
      },
      {
        path: 'edit/:fsname',
        component: FilesystemEditFormPageComponent,
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated file system settings.')
        }
      },
      {
        path: 'quota/:uuid',
        data: { title: gettext('Quota') },
        children: [
          { path: '', component: FilesystemQuotaDatatablePageComponent },
          {
            path: 'edit/:type/:name',
            component: FilesystemQuotaFormPageComponent,
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated quota.')
            }
          }
        ]
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
        routeConfigService.inject('storage', routes);
        return routes;
      },
      deps: [RouteConfigService]
    }
  ]
})
export class StorageRoutingModule {}
