import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class FilesystemCreateFormPageComponent {
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
