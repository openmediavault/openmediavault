/* eslint-disable max-len */
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { CoreModule } from '~/app/core/core.module';
import { MaterialModule } from '~/app/material.module';
import { DiskDatatablePageComponent } from '~/app/pages/storage/disks/disk-datatable-page.component';
import { DiskFormPageComponent } from '~/app/pages/storage/disks/disk-form-page.component';
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
import { SharedFolderSnapshotsDatatablePageComponent } from '~/app/pages/storage/shared-folders/shared-folder-snapshots-datatable-page.component';
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
    MdDatatablePageComponent,
    MdFormPageComponent,
    MdDetailsTextPageComponent,
    MdGrowFormPageComponent,
    MdRemoveFormPageComponent,
    MdRecoverFormPageComponent,
    SharedFolderFormPageComponent,
    SharedFolderPrivilegesDatatablePageComponent,
    SharedFolderSnapshotsDatatablePageComponent,
    FilesystemEditFormPageComponent,
    FilesystemMountFormPageComponent,
    FilesystemQuotaDatatablePageComponent,
    FilesystemQuotaFormPageComponent,
    SharedFolderAclFormPageComponent
  ],
  imports: [CommonModule, CoreModule, MaterialModule, SharedModule, StorageRoutingModule]
})
export class StorageModule {}
