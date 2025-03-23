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
export class SshFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'SSH',
      get: {
        method: 'get'
      },
      post: {
        method: 'set'
      }
    },
    hints: [
      {
        type: 'info',
        text: gettext(
          'Users must be assigned to the <em>_ssh</em> group to be able to log in via SSH.'
        ),
        dismissible: true,
        stateId: '1f7e0754-e049-4578-9272-8cbb365fad97'
      }
    ],
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
        value: 22,
        validators: {
          min: 0,
          max: 65535,
          patternType: 'port',
          requiredIf: { operator: 'eq', arg0: { prop: 'enable' }, arg1: true }
        }
      },
      {
        type: 'checkbox',
        name: 'permitrootlogin',
        label: gettext('Permit root login'),
        value: true,
        hint: gettext('Specifies whether it is allowed to login as superuser.')
      },
      {
        type: 'checkbox',
        name: 'passwordauthentication',
        label: gettext('Password authentication'),
        value: true,
        hint: gettext('Enable keyboard-interactive authentication.')
      },
      {
        type: 'checkbox',
        name: 'pubkeyauthentication',
        label: gettext('Public key authentication'),
        value: true,
        hint: gettext('Enable public key authentication.')
      },
      {
        type: 'checkbox',
        name: 'tcpforwarding',
        label: gettext('TCP forwarding'),
        value: true,
        hint: gettext('Permit to do SSH tunneling.')
      },
      {
        type: 'checkbox',
        name: 'compression',
        label: gettext('Compression'),
        value: true,
        hint: gettext(
          'Compression is worth using if your connection is slow. The efficiency of the compression depends on the type of the file, and varies widely. Useful for internet transfer only.'
        )
      },
      {
        type: 'textarea',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          'Please check the <a href="https://man.openbsd.org/sshd_config.5" target="_blank">manual page</a> for more details.'
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
          url: '/services'
        }
      }
    ]
  };
}
