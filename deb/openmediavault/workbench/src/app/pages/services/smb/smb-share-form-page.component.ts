/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class SmbShareFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'SMB',
      get: {
        method: 'getShare',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setShare'
      }
    },
    fields: [
      /* eslint-disable max-len */
      {
        type: 'confObjUuid'
      },
      {
        type: 'checkbox',
        name: 'enable',
        label: gettext('Enabled'),
        value: true
      },
      {
        type: 'sharedFolderSelect',
        name: 'sharedfolderref',
        label: gettext('Shared folder'),
        hint: gettext('The location of the files to share.'),
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'comment',
        label: gettext('Comment'),
        value: '',
        hint: gettext(
          'This is a text field that is seen next to a share when a client queries the server.'
        )
      },
      {
        type: 'select',
        name: 'guest',
        label: gettext('Public'),
        hint: gettext(
          "If 'Guests allowed' is selected and no login credential is provided, then access as guest. Always access as guest when 'Guests only' is selecting; in this case no password is required to connect to the share." // eslint-disable-line @typescript-eslint/quotes
        ),
        value: 'no',
        store: {
          data: [
            ['no', gettext('No')],
            ['allow', gettext('Guests allowed')],
            ['only', gettext('Guests only')]
          ]
        }
      },
      {
        type: 'checkbox',
        name: 'readonly',
        label: gettext('Read-only'),
        hint: gettext(
          'If this parameter is set, then users may not create or modify files in the share.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'browseable',
        label: gettext('Browseable'),
        hint: gettext(
          'This controls whether this share is seen in the list of available shares in a net view and in the browse list.'
        ),
        value: true
      },
      {
        type: 'checkbox',
        name: 'timemachine',
        label: gettext('Time Machine support'),
        hint: gettext('Enable Time Machine support for this share.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'inheritacls',
        label: gettext('Inherit ACLs'),
        hint: gettext(
          'This parameter can be used to ensure that if default acls exist on parent directories, they are always honored when creating a new file or subdirectory in these parent directories.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'inheritpermissions',
        label: gettext('Inherit permissions'),
        hint: gettext(
          'The permissions on new files and directories are normally governed by create mask and directory mask but the inherit permissions parameter overrides this. This can be particularly useful on systems with many users to allow a single share to be used flexibly by each user.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'recyclebin',
        label: gettext('Enable recycle bin'),
        hint: gettext('This will create a recycle bin on the share.'),
        value: false
      },
      {
        type: 'container',
        fields: [
          {
            type: 'numberInput',
            name: 'recyclemaxsize',
            label: gettext('Maximum file size'),
            hint: gettext(
              'Files that are larger than the specified number of bytes will not be put into the recycle bin. Set to 0 for unrestricted file size.'
            ),
            value: 0,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'falsy', arg0: { prop: 'recyclebin' } }
              }
            ],
            validators: {
              min: 0,
              required: true
            },
            flex: 45
          },
          {
            type: 'numberInput',
            name: 'recyclemaxage',
            label: gettext('Retention time'),
            hint: gettext(
              'Files in the recycle bin will be deleted automatically after the specified number of days. Set to 0 for manual deletion.'
            ),
            value: 0,
            modifiers: [
              {
                type: 'disabled',
                constraint: { operator: 'falsy', arg0: { prop: 'recyclebin' } }
              }
            ],
            validators: {
              min: 0,
              required: true
            },
            flex: 45
          },
          {
            type: 'iconButton',
            name: 'recyclenow',
            label: gettext('Empty now'),
            icon: 'eraser',
            submitValue: false,
            request: {
              service: 'SMB',
              method: 'emptyRecycleBin',
              params: {
                uuid: '{{ _routeParams.uuid }}'
              },
              successNotification: gettext('Cleaning up recycle bin on the share.')
            },
            modifiers: [
              {
                type: 'disabled',
                constraint: {
                  operator: 'or',
                  arg0: {
                    operator: 'falsy',
                    arg0: { value: '{{ _routeConfig.data.editing | toboolean }}' }
                  },
                  arg1: { operator: 'falsy', arg0: { prop: 'recyclebin' } }
                }
              }
            ]
          }
        ]
      },
      {
        type: 'checkbox',
        name: 'hidedotfiles',
        label: gettext('Hide dot files'),
        hint: gettext(
          'This parameter controls whether files starting with a dot appear as hidden files'
        ),
        value: true
      },
      {
        type: 'checkbox',
        name: 'easupport',
        label: gettext('Extended attributes'),
        hint: gettext(
          'Allow clients to attempt to store OS/2 state extended attributes on a share.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'storedosattributes',
        label: gettext('Store DOS attributes'),
        hint: gettext(
          'If this parameter is set, Samba attempts to first read DOS attributes (SYSTEM, HIDDEN, ARCHIVE or READ-ONLY) from a file system extended attribute, before mapping DOS attributes to UNIX permission bits. When set, DOS attributes will be stored onto an extended attribute in the UNIX file system, associated with the file or directory.'
        ),
        value: false
      },
      {
        type: 'textInput',
        name: 'hostsallow',
        label: gettext('Hosts allow'),
        hint: gettext(
          'This option is a comma, space, or tab delimited set of hosts which are permitted to access this share. You can specify the hosts by name or IP number. Leave this field empty to use default settings.'
        ),
        value: ''
      },
      {
        type: 'textInput',
        name: 'hostsdeny',
        label: gettext('Hosts deny'),
        hint: gettext(
          'This option is a comma, space, or tab delimited set of host which are NOT permitted to access this share. Where the lists conflict, the allow list takes precedence. In the event that it is necessary to deny all by default, use the keyword ALL (or the netmask 0.0.0.0/0) and then explicitly specify to the hosts allow parameter those hosts that should be permitted access. Leave this field empty to use default settings.'
        ),
        value: ''
      },
      {
        type: 'checkbox',
        name: 'audit',
        label: gettext('Audit file operations'),
        value: false
      },
      {
        type: 'textarea',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          "Please check the <a href='http://www.samba.org/samba/docs/man/manpages-3/smb.conf.5.html' target='_blank'>manual page</a> for more details." // eslint-disable-line @typescript-eslint/quotes
        ),
        value: ''
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/services/smb/shares'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/services/smb/shares'
        }
      }
    ]
  };
}
