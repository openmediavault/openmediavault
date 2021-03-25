import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class MdFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'RaidMgmt',
      post: {
        method: 'create'
      }
    },
    fields: [
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        value: ''
      },
      {
        type: 'select',
        name: 'level',
        label: gettext('Level'),
        value: 'raid5',
        store: {
          data: [
            ['stripe', gettext('Stripe')],
            ['mirror', gettext('Mirror')],
            ['linear', gettext('Linear')],
            ['raid10', gettext('RAID 10')],
            ['raid5', gettext('RAID 5')],
            ['raid6', gettext('RAID 6')]
          ]
        }
      },
      {
        type: 'select',
        name: 'devices',
        label: gettext('Devices'),
        placeholder: gettext('Select devices ...'),
        hint: gettext(
          'Select the devices that will be used to create the RAID device. Devices connected via USB will not be listed (too unreliable).'
        ),
        multiple: true,
        valueField: 'devicefile',
        textField: 'description',
        store: {
          proxy: {
            service: 'RaidMgmt',
            get: {
              method: 'getCandidates'
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
          required: true,
          custom: [
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'in',
                  arg0: { prop: 'level' },
                  arg1: ['stripe', 'linear', 'mirror']
                },
                arg1: {
                  operator: '>=',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'devices' }
                  },
                  arg1: 2
                }
              },
              errorData: gettext('At least two devices are required.')
            },
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'eq',
                  arg0: { prop: 'level' },
                  arg1: 'raid5'
                },
                arg1: {
                  operator: '>=',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'devices' }
                  },
                  arg1: 3
                }
              },
              errorData: gettext('At least three devices are required.')
            },
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'in',
                  arg0: { prop: 'level' },
                  arg1: ['raid6', 'raid10']
                },
                arg1: {
                  operator: '>=',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'devices' }
                  },
                  arg1: 4
                }
              },
              errorData: gettext('At least four devices are required.')
            }
          ]
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/storage/md'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/md'
        }
      }
    ]
  };
}
