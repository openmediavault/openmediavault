/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import { IsDirtyFormPageComponent } from '~/app/pages/is-dirty-page-component';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class MdFormPageComponent extends IsDirtyFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'RaidMgmt',
      post: {
        method: 'create'
      }
    },
    fields: [
      {
        type: 'hidden',
        name: 'name',
        label: gettext('Name'),
        value: '',
        validators: {
          patternType: 'wordChars'
        }
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
