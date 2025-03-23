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
export class UpdateSettingsSoftwareFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Apt',
      get: {
        method: 'getSoftwareSettings'
      },
      post: {
        method: 'setSoftwareSettings'
      }
    },
    fields: [
      {
        type: 'paragraph',
        title: gettext('Install updates from')
      },
      {
        type: 'checkbox',
        name: 'proposed',
        label: gettext('Pre-release updates'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'partner',
        label: gettext('Community-maintained updates'),
        value: false
      }
    ],
    buttons: [
      {
        template: 'submit',
        confirmationDialogConfig: {
          template: 'confirmation',
          message: gettext(
            'The information about available software is out-of-date. Do you want to reload this information?'
          )
        },
        execute: {
          type: 'request',
          request: {
            service: 'Apt',
            method: 'update',
            task: true,
            progressMessage: gettext(
              'The repository will be checked for new, removed or upgraded software packages.'
            ),
            successUrl: '/system/updatemgmt/updates'
          }
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/updatemgmt/settings'
        }
      }
    ]
  };
}
