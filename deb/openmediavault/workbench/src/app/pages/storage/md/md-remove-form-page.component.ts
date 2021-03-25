import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class MdRemoveFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'RaidMgmt',
      get: {
        method: 'get',
        params: {
          devicefile: '{{ _routeParams.devicefile }}'
        },
        filter: {
          mode: 'pick',
          props: ['description', 'devicefile', 'level']
        }
      },
      post: {
        method: 'remove'
      }
    },
    fields: [
      {
        type: 'textInput',
        name: 'description',
        label: gettext('Device'),
        disabled: true,
        submitValue: false
      },
      {
        type: 'hidden',
        name: 'devicefile',
        value: '{{ _routeParams.devicefile }}'
      },
      {
        type: 'hidden',
        name: 'level',
        submitValue: false
      },
      {
        type: 'select',
        name: 'devices',
        label: gettext('Devices'),
        placeholder: gettext('Select devices ...'),
        hint: gettext('Select devices to be removed from the RAID device.'),
        multiple: true,
        valueField: 'devicefile',
        textField: 'description',
        store: {
          proxy: {
            service: 'RaidMgmt',
            get: {
              method: 'getSlaves',
              params: {
                devicefile: '{{ _routeParams.devicefile }}'
              }
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
          // Set the max. number of devices that can be removed.
          // See for fault tolerance at:
          // http://en.wikipedia.org/wiki/Standard_RAID_levels
          custom: [
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: 'in',
                  arg0: { prop: 'level' },
                  arg1: ['mirror', 'raid1', 'raid2', 'raid3', 'raid4', 'raid5', 'raid10']
                },
                arg1: {
                  operator: '<=',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'devices' }
                  },
                  arg1: 1
                }
              },
              errorData: gettext('A maximum of one device can be removed.')
            },
            {
              constraint: {
                operator: 'if',
                arg0: {
                  operator: '===',
                  arg0: { prop: 'level' },
                  arg1: 'raid6'
                },
                arg1: {
                  operator: '<=',
                  arg0: {
                    operator: 'length',
                    arg0: { prop: 'devices' }
                  },
                  arg1: 2
                }
              },
              errorData: gettext('A maximum of two device can be removed.')
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
