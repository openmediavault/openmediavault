import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class FilesystemFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'FileSystemMgmt',
      post: {
        method: 'create',
        confirmationDialogConfig: {
          template: 'confirmation-danger',
          message: gettext(
            // eslint-disable-next-line max-len
            'Do you really want to create the file system? All data on the device will be deleted. Please note that the file system creation may take some time.'
          )
        }
      }
    },
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
        type: 'textInput',
        name: 'label',
        label: gettext('Label'),
        hint: gettext('The label of the file system.'),
        value: '',
        validators: {
          maxLength: 16,
          pattern: {
            pattern: '[a-zA-Z0-9]+',
            errorData: gettext('The value may only contain letters and numbers.')
          },
          custom: [
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'eq',
                  arg0: { prop: 'type' },
                  arg1: 'xfs'
                },
                arg1: {
                  operator: '<=',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'label' }
                  },
                  arg1: 12
                }
              },
              errorData: gettext('The label cannot exceed 12 characters.')
            }
          ]
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
