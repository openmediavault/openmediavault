/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class FilesystemMountFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'FileSystemMgmt',
      post: {
        method: 'setMountPoint',
        confirmationDialogConfig: {
          template: 'confirmation',
          message: gettext('Do you really want to mount the file system?')
        },
        progressMessage: gettext('Please wait, the file system is being mounted.')
      }
    },
    fields: [
      {
        type: 'select',
        name: 'id',
        label: gettext('Filesystem'),
        placeholder: gettext('Select a file system ...'),
        hint: gettext('The file system to mount.'),
        validators: {
          required: true
        },
        valueField: 'devicefile',
        textField: 'description',
        store: {
          proxy: {
            service: 'FileSystemMgmt',
            get: {
              method: 'getMountCandidates'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'devicefile'
            }
          ]
        }
      },
      {
        type: 'numberInput',
        name: 'usagewarnthreshold',
        label: gettext('Usage Warning Threshold'),
        value: 85,
        validators: {
          required: true,
          min: 0,
          max: 100
        },
        hint: gettext(
          // eslint-disable-next-line max-len
          'Send a notification when the used file system capacity exceeds the specified threshold (in percent). Set to 0 to disable this notification.'
        )
      },
      {
        type: 'textInput',
        name: 'comment',
        label: gettext('Comment'),
        value: ''
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/filesystems'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/filesystems'
        }
      }
    ]
  };
}
