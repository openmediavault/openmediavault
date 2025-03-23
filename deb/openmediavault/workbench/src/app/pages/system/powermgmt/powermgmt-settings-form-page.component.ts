/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
export class PowermgmtSettingsFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'PowerMgmt',
      get: {
        method: 'get'
      },
      post: {
        method: 'set'
      }
    },
    fields: [
      {
        type: 'checkbox',
        name: 'cpufreq',
        label: gettext('Monitoring'),
        hint: gettext(
          'Specifies whether to monitor the system status and select the most appropriate CPU level.'
        ),
        value: false
      },
      {
        type: 'select',
        name: 'powerbtn',
        label: gettext('Power button'),
        hint: gettext('The action to be done when pressing the power button.'),
        value: 'nothing',
        store: {
          data: [
            ['nothing', gettext('Nothing')],
            ['shutdown', gettext('Power Off')],
            ['standby', gettext('Standby')]
          ]
        }
      },
      {
        type: 'select',
        name: 'standbymode',
        label: gettext('Standby mode'),
        value: 'poweroff',
        store: {
          proxy: {
            service: 'PowerMgmt',
            get: {
              method: 'enumerateStandbyModes'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'text'
            }
          ],
          assign: {
            key: 'value',
            sources: {
              poweroff: { text: gettext('Power Off') },
              suspend: { text: gettext('Suspend') },
              hibernate: { text: gettext('Hibernate') },
              suspendhybrid: { text: gettext('Hybrid sleep') }
            }
          }
        }
      }
    ],
    buttons: [
      {
        template: 'submit'
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/powermgmt'
        }
      }
    ]
  };
}
