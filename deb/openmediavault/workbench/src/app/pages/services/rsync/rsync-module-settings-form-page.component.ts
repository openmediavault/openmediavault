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
export class RsyncModuleSettingsFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Rsyncd',
      get: {
        method: 'getSettings'
      },
      post: {
        method: 'setSettings'
      }
    },
    fields: [
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: false
      },
      {
        type: 'numberInput',
        name: 'port',
        label: gettext('Port'),
        value: 873,
        validators: {
          min: 1,
          max: 65535,
          required: true,
          patternType: 'port'
        }
      },
      {
        type: 'textarea',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          'Please check the <a href="http://www.samba.org/ftp/rsync/rsyncd.conf.html" target="_blank">manual page</a> for more details.'
        ),
        value: ''
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
          url: '/services/rsync/server'
        }
      }
    ]
  };
}
