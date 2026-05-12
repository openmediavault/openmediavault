/* eslint-disable max-len */
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CoreModule } from '~/app/core/core.module';
import { MaterialModule } from '~/app/material.module';
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
import { StorageRoutingModule } from '~/app/pages/storage/storage-routing.module';
import { SharedModule } from '~/app/shared/shared.module';

@NgModule({
  declarations: [
    DiskDatatablePageComponent,
    DiskFormPageComponent,
    SharedFolderDatatablePageComponent,
    FilesystemDatatablePageComponent,
    SmartDeviceDatatablePageComponent,
    SmartSettingsFormPageComponent,
    SmartTaskDatatablePageComponent,
    SmartTaskFormPageComponent,
    SmartDeviceFormPageComponent,
    SmartDeviceDetailsTabsPageComponent,
    SharedFolderFormPageComponent,
    SharedFolderPermissionsDatatablePageComponent,
    SharedFolderSnapshotsTabsPageComponent,
    SharedFolderAllSnapshotsTabsPageComponent,
    FilesystemDetailsTextPageComponent,
    FilesystemEditFormPageComponent,
    FilesystemMountFormPageComponent,
    FilesystemQuotaDatatablePageComponent,
    FilesystemQuotaFormPageComponent,
    SharedFolderAclFormPageComponent
  ],
  imports: [CommonModule, CoreModule, MaterialModule, SharedModule, StorageRoutingModule]
})
export class StorageModule {}
