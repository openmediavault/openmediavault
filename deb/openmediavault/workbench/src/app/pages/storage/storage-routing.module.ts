/* eslint-disable max-len */
import { NgModule } from '@angular/core';
import { RouterModule, ROUTES, Routes } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { RouteConfigService } from '~/app/core/services/route-config.service';
import { DiskDatatablePageComponent } from '~/app/pages/storage/disks/disk-datatable-page.component';
import { DiskFormPageComponent } from '~/app/pages/storage/disks/disk-form-page.component';
import { FilesystemDatatablePageComponent } from '~/app/pages/storage/filesystems/filesystem-datatable-page.component';
import { FilesystemDetailsTextPageComponent } from '~/app/pages/storage/filesystems/filesystem-details-text-page.component';
import { FilesystemEditFormPageComponent } from '~/app/pages/storage/filesystems/filesystem-edit-form-page.component';
import { FilesystemMountFormPageComponent } from '~/app/pages/storage/filesystems/filesystem-mount-form-page.component';
import { FilesystemQuotaDatatablePageComponent } from '~/app/pages/storage/filesystems/filesystem-quota-datatable-page.component';
import { FilesystemQuotaFormPageComponent } from '~/app/pages/storage/filesystems/filesystem-quota-form-page.component';
import { SharedFolderAclFormPageComponent } from '~/app/pages/storage/shared-folders/shared-folder-acl-form-page.component';
import { SharedFolderAllSnapshotsTabsPageComponent } from '~/app/pages/storage/shared-folders/shared-folder-all-snapshots-tabs-page.component';
import { SharedFolderDatatablePageComponent } from '~/app/pages/storage/shared-folders/shared-folder-datatable-page.component';
import { SharedFolderFormPageComponent } from '~/app/pages/storage/shared-folders/shared-folder-form-page.component';
import { SharedFolderPermissionsDatatablePageComponent } from '~/app/pages/storage/shared-folders/shared-folder-permissions-datatable-page.component';
import { SharedFolderSnapshotsTabsPageComponent } from '~/app/pages/storage/shared-folders/shared-folder-snapshots-tabs-page.component';
import { SmartDeviceDatatablePageComponent } from '~/app/pages/storage/smart/smart-device-datatable-page.component';
import { SmartDeviceDetailsTabsPageComponent } from '~/app/pages/storage/smart/smart-device-details-tabs-page.component';
import { SmartDeviceFormPageComponent } from '~/app/pages/storage/smart/smart-device-form-page.component';
import { SmartSettingsFormPageComponent } from '~/app/pages/storage/smart/smart-settings-form-page.component';
import { SmartTaskDatatablePageComponent } from '~/app/pages/storage/smart/smart-task-datatable-page.component';
import { SmartTaskFormPageComponent } from '~/app/pages/storage/smart/smart-task-form-page.component';
import { IsDirtyGuardService } from '~/app/shared/services/is-dirty-guard.service';

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
        canDeactivate: [IsDirtyGuardService],
        data: { title: gettext('Edit'), editing: false }
      },
      {
        path: 'hdparm/edit/:uuid',
        component: DiskFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
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
        canDeactivate: [IsDirtyGuardService],
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
            canDeactivate: [IsDirtyGuardService],
            data: { title: gettext('Edit'), editing: false }
          },
          {
            path: 'edit/:uuid/:devicefile',
            component: SmartDeviceFormPageComponent,
            canDeactivate: [IsDirtyGuardService],
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
            canDeactivate: [IsDirtyGuardService],
            data: { title: gettext('Create'), editing: false }
          },
          {
            path: 'edit/:uuid',
            component: SmartTaskFormPageComponent,
            canDeactivate: [IsDirtyGuardService],
            data: { title: gettext('Edit'), editing: true }
          }
        ]
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
        canDeactivate: [IsDirtyGuardService],
        data: { title: gettext('Create'), editing: false }
      },
      {
        path: 'edit/:uuid',
        component: SharedFolderFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('Edit'),
          editing: true,
          notificationTitle: gettext('Updated shared folder.')
        }
      },
      {
        path: 'permissions/:uuid',
        component: SharedFolderPermissionsDatatablePageComponent,
        data: {
          title: gettext('Permissions'),
          breadcrumb: {
            text: '{{ "Permissions" | translate }} @ {{ name }}',
            request: {
              service: 'ShareMgmt',
              method: 'get',
              params: { uuid: '{{ _routeParams.uuid }}' }
            }
          },
          notificationTitle: gettext('Updated permissions of shared folder.')
        }
      },
      {
        path: 'acl/:uuid',
        component: SharedFolderAclFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: {
          title: gettext('ACL'),
          notificationTitle: gettext('Updated access control list of shared folder.')
        }
      },
      {
        path: 'snapshots',
        component: SharedFolderAllSnapshotsTabsPageComponent,
        data: {
          title: gettext('All Snapshots'),
          editing: true
        }
      },
      {
        path: 'snapshots/:uuid',
        component: SharedFolderSnapshotsTabsPageComponent,
        data: {
          title: gettext('Snapshots'),
          breadcrumb: {
            text: '{{ "Snapshots" | translate }} @ {{ name }}',
            request: {
              service: 'ShareMgmt',
              method: 'get',
              params: { uuid: '{{ _routeParams.uuid }}' }
            }
          }
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
        path: 'mount',
        component: FilesystemMountFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
        data: { title: gettext('Mount'), editing: false }
      },
      {
        path: 'edit/:fsname',
        component: FilesystemEditFormPageComponent,
        canDeactivate: [IsDirtyGuardService],
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
            canDeactivate: [IsDirtyGuardService],
            data: {
              title: gettext('Edit'),
              editing: true,
              notificationTitle: gettext('Updated quota.')
            }
          }
        ]
      },
      {
        path: 'details/:devicefile',
        component: FilesystemDetailsTextPageComponent,
        data: {
          title: gettext('Details'),
          breadcrumb: {
            text: '{{ "Details" | translate }} @ {{ _routeParams.devicefile }}'
          }
        }
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
