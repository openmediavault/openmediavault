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
export class SmbSettingsFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'SMB',
      get: {
        method: 'getSettings'
      },
      post: {
        method: 'setSettings'
      }
    },
    fields: [
      /* eslint-disable max-len */
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: false
      },
      {
        type: 'textInput',
        name: 'workgroup',
        label: gettext('Workgroup'),
        value: 'WORKGROUP',
        hint: gettext('The workgroup the server will appear to be in when queried by clients.'),
        validators: {
          required: true,
          patternType: 'netbiosName'
        }
      },
      {
        type: 'textInput',
        name: 'serverstring',
        label: gettext('Description'),
        value: '%h server',
        hint: gettext('The NT description field.'),
        validators: {
          required: true
        }
      },
      {
        type: 'checkbox',
        name: 'timeserver',
        label: gettext('Time server'),
        hint: gettext('Allow this server to advertise itself as a time server to Windows clients.'),
        value: false
      },
      {
        type: 'divider',
        title: gettext('Home directories')
      },
      {
        type: 'checkbox',
        name: 'homesenable',
        label: gettext('Enabled'),
        hint: gettext('Enable user home directories.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'homesbrowseable',
        label: gettext('Browseable'),
        hint: gettext(
          'This controls whether this share is seen in the list of available shares in a net view and in the browse list.'
        ),
        value: true
      },
      {
        type: 'checkbox',
        name: 'homesinheritacls',
        label: gettext('Inherit ACLs'),
        hint: gettext(
          'This parameter can be used to ensure that if default acls exist on parent directories, they are always honored when creating a new file or subdirectory in these parent directories.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'homesinheritpermissions',
        label: gettext('Inherit permissions'),
        hint: gettext(
          'The permissions on new files and directories are normally governed by create mask and directory mask but the inherit permissions parameter overrides this. This can be particularly useful on systems with many users to allow a single share to be used flexibly by each user.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'homesrecyclebin',
        label: gettext('Enable recycle bin'),
        hint: gettext('This will create a recycle bin for each user home directory.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'homesfollowsymlinks',
        label: gettext('Follow symlinks'),
        hint: gettext('Allow following symbolic links in the home directories.'),
        value: true,
        modifiers: [
          {
            type: 'checked',
            opposite: false,
            constraint: { operator: 'truthy', arg0: { prop: 'homeswidelinks' } }
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'homeswidelinks',
        label: gettext('Wide links'),
        hint: gettext('Allow symbolic links to areas that are outside the home directories.'),
        value: false,
        modifiers: [
          {
            type: 'unchecked',
            opposite: false,
            constraint: { operator: 'falsy', arg0: { prop: 'homesfollowsymlinks' } }
          }
        ]
      },
      {
        type: 'textarea',
        name: 'homesextraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          "Please check the <a href='http://www.samba.org/samba/docs/man/manpages-3/smb.conf.5.html' target='_blank'>manual page</a> for more details." // eslint-disable-line @typescript-eslint/quotes
        ),
        value: ''
      },
      {
        type: 'divider',
        title: gettext('Advanced settings')
      },
      {
        type: 'select',
        name: 'minprotocol',
        label: gettext('Minimum protocol version'),
        hint:
          gettext(
            'This setting controls the minimum protocol version that the server will allow the client to use.'
          ) +
          ' ' +
          gettext('Note that SMB1 is deprecated and should only be used in mandatory cases.'),
        value: 'SMB2',
        store: {
          data: [
            ['SMB1', 'SMB1'],
            ['SMB2', 'SMB2'],
            ['SMB3', 'SMB3']
          ]
        }
      },
      {
        type: 'checkbox',
        name: 'netbios',
        label: gettext('Enable NetBIOS'),
        hint: gettext('Support name resolution and network discovery for legacy systems.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'winssupport',
        label: gettext('Enable WINS server'),
        hint: gettext('Act as a WINS server.'),
        value: false,
        validators: {
          custom: [
            {
              constraint: {
                operator: 'or',
                arg0: { operator: 'empty', arg0: { prop: 'winsserver' } },
                arg1: { operator: 'falsy', arg0: { prop: 'winssupport' } }
              },
              errorData: gettext(
                'Acting as a WINS server and using a WINS server are mutually exclusive.'
              )
            }
          ]
        }
      },
      {
        type: 'textInput',
        name: 'winsserver',
        label: gettext('WINS server'),
        value: '',
        hint: gettext('Use the specified WINS server.'),
        modifiers: [
          {
            type: 'disabled',
            constraint: { operator: 'truthy', arg0: { prop: 'winssupport' } }
          }
        ],
        validators: {
          custom: [
            {
              constraint: {
                operator: 'or',
                arg0: { operator: 'empty', arg0: { prop: 'winsserver' } },
                arg1: { operator: 'falsy', arg0: { prop: 'winssupport' } }
              },
              errorData: gettext(
                'Acting as a WINS server and using a WINS server are mutually exclusive.'
              )
            }
          ]
        }
      },
      {
        type: 'checkbox',
        name: 'usesendfile',
        label: gettext('Use sendfile'),
        hint: gettext(
          "Use the more efficient sendfile system call for files that are exclusively oplocked. This may make more efficient use of the system CPU's and cause Samba to be faster. Samba automatically turns this off for clients that use protocol levels lower than NT LM 0.12 and when it detects a client is Windows 9x."
        ),
        value: true
      },
      {
        type: 'checkbox',
        name: 'aio',
        label: gettext('Asynchronous I/O'),
        value: true
      },
      {
        type: 'select',
        name: 'loglevel',
        label: gettext('Log level'),
        value: 0,
        store: {
          data: [
            [0, gettext('None')],
            [1, gettext('Minimum')],
            [2, gettext('Normal')],
            [3, gettext('Full')],
            [10, gettext('Debug')]
          ]
        }
      },
      {
        type: 'textarea',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          "Please check the <a href='http://www.samba.org/samba/docs/man/manpages-3/smb.conf.5.html' target='_blank'>manual page</a> for more details."
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
          url: '/services/smb'
        }
      }
    ]
  };
}
