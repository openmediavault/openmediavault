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
export class UserFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'UserMgmt',
      get: {
        method: 'getUser',
        params: {
          name: '{{ _routeParams.name }}'
        }
      },
      post: {
        method: 'setUser'
      }
    },
    fields: [
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        disabled: '{{ _routeConfig.data.editing | toboolean }}',
        value: '{{ _routeParams.name }}',
        autocomplete: 'off',
        validators: {
          required: true,
          patternType: 'userName'
        }
      },
      {
        type: 'textInput',
        name: 'email',
        label: gettext('Email'),
        value: '',
        autocomplete: 'off',
        validators: {
          patternType: 'email'
        }
      },
      {
        type: 'passwordInput',
        name: 'password',
        label: gettext('Password'),
        value: '',
        autocomplete: 'new-password',
        validators: {
          required: '{{ _routeConfig.data.editing | toboolean === false }}'
        }
      },
      {
        type: 'passwordInput',
        name: 'passwordconf',
        label: gettext('Confirm password'),
        submitValue: false,
        value: '',
        autocomplete: 'new-password',
        validators: {
          custom: [
            {
              constraint: {
                operator: 'eq',
                arg0: { prop: 'password' },
                arg1: { prop: 'passwordconf' }
              },
              errorData: gettext('Password does not match.')
            }
          ]
        }
      },
      {
        type: 'select',
        name: 'shell',
        label: gettext('Shell'),
        placeholder: gettext('Select a shell ...'),
        value: '/usr/bin/sh',
        store: {
          proxy: {
            service: 'System',
            get: {
              method: 'getShells'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'text'
            }
          ]
        }
      },
      {
        type: 'select',
        name: 'groups',
        label: gettext('Groups'),
        placeholder: gettext('Select groups ...'),
        multiple: true,
        valueField: 'name',
        textField: 'name',
        value: [],
        store: {
          proxy: {
            service: 'UserMgmt',
            get: {
              method: 'enumerateAllGroups'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'name'
            }
          ]
        }
      },
      {
        type: 'datatable',
        name: 'sshpubkeys',
        label: gettext('SSH public keys'),
        hasHeader: false,
        hasFooter: false,
        columns: [
          {
            prop: 'sshpubkey',
            flexGrow: 1,
            cellClass: 'omv-text-wrap omv-text-monospace'
          }
        ],
        actions: [
          {
            template: 'add',
            formDialogConfig: {
              title: gettext('SSH public key'),
              fields: [
                {
                  type: 'fileInput',
                  name: 'sshpubkey',
                  value: '',
                  rows: 10,
                  wrap: 'off',
                  monospace: true,
                  trim: true,
                  label: gettext('Public key'),
                  hint: gettext('The SSH public key in OpenSSH or RFC 4716 format.'),
                  validators: {
                    required: true,
                    patternType: 'sshPubKey'
                  }
                }
              ]
            }
          },
          {
            template: 'delete'
          }
        ],
        valueType: 'string',
        value: []
      },
      {
        type: 'checkbox',
        name: 'disallowusermod',
        label: gettext('Disallow account modification'),
        value: false,
        hint: gettext('Disallow the user to modify their own account.')
      },
      {
        type: 'tagInput',
        name: 'comment',
        label: gettext('Tags'),
        value: '',
        validators: {
          maxLength: 65
        }
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/usermgmt/users'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/usermgmt/users'
        }
      }
    ]
  };
}
