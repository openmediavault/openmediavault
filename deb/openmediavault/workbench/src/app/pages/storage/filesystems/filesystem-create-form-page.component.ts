/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2023 Volker Theile
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
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';
import { BaseFormPageComponent } from '~/app/pages/base-page-component';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class FilesystemCreateFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    fields: [
      {
        type: 'select',
        name: 'devicefile',
        label: gettext('Device'),
        placeholder: gettext('Select a device ...'),
        hint: gettext('Select the device that will be used to create the file system.'),
        valueField: 'devicefile',
        textField: 'description',
        store: {
          proxy: {
            service: 'FileSystemMgmt',
            get: {
              method: 'getCandidatesBg',
              task: true
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'devicefile'
            }
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'select',
        name: 'type',
        label: gettext('Type'),
        placeholder: gettext('Select a type ...'),
        hint: gettext('The type of the file system to create.'),
        value: 'ext4',
        store: {
          data: [
            ['btrfs', gettext('BTRFS')],
            ['ext3', gettext('EXT3')],
            ['ext4', gettext('EXT4')],
            ['f2fs', gettext('F2FS')],
            ['xfs', gettext('XFS')],
            ['jfs', gettext('JFS')]
          ],
          sorters: [
            {
              dir: 'asc',
              prop: 'value'
            }
          ]
        },
        validators: {
          required: true
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'taskDialog',
          taskDialog: {
            config: {
              title: gettext('Create file system'),
              width: '75%',
              startOnInit: true,
              request: {
                service: 'FileSystemMgmt',
                method: 'create',
                params: {
                  devicefile: '{{ devicefile }}',
                  type: '{{ type }}'
                }
              },
              buttons: {
                stop: {
                  hidden: true
                }
              }
            },
            successUrl: '/storage/filesystems/mount'
          }
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
