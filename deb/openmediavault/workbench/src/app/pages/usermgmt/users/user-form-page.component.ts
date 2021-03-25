import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class UserFormPageComponent {
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
        value: '/bin/sh',
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
              prop: 'label'
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
            cellClass: 'omv-text-monospaced'
          }
        ],
        actions: [
          {
            template: 'add',
            dialogConfig: {
              title: gettext('SSH public key'),
              fields: [
                {
                  type: 'textarea',
                  name: 'sshpubkey',
                  value: '',
                  rows: 10,
                  monospaced: true,
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
        hint: gettext('Disallow the user to modify his own account.')
      },
      {
        type: 'textInput',
        name: 'comment',
        label: gettext('Comment'),
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
