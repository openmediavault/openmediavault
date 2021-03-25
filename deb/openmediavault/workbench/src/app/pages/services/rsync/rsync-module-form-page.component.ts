import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { FormPageConfig } from '~/app/core/components/limn-ui/models/form-page-config.type';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class RsyncModuleFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'Rsyncd',
      get: {
        method: 'getModule',
        params: {
          uuid: '{{ _routeParams.uuid }}'
        }
      },
      post: {
        method: 'setModule'
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
          requiredIf: { operator: 'eq', arg0: { prop: 'enable' }, arg1: true }
        }
      },
      {
        type: 'textInput',
        name: 'name',
        label: gettext('Name'),
        value: '',
        hint: gettext('The name of the share.'),
        validators: {
          required: true
        }
      },
      {
        type: 'select',
        name: 'uid',
        label: gettext('User'),
        placeholder: gettext('Select an user ...'),
        hint: gettext(
          'This option specifies the user name that file transfers to and from that module should take place.'
        ),
        value: 'nobody',
        valueField: 'name',
        textField: 'name',
        store: {
          proxy: {
            service: 'UserMgmt',
            get: {
              method: 'enumerateAllUsers'
            }
          },
          sorters: [
            {
              dir: 'asc',
              prop: 'name'
            }
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'select',
        name: 'gid',
        label: gettext('Group'),
        placeholder: gettext('Select a group ...'),
        hint: gettext(
          'This option specifies the group name that file transfers to and from that module should take place.'
        ),
        value: 'users',
        valueField: 'name',
        textField: 'name',
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
        },
        validators: {
          required: true
        }
      },
      {
        type: 'datatable',
        name: 'users',
        label: gettext('Users'),
        hint: gettext('The users that are allowed to access this module.'),
        columns: [
          {
            name: gettext('Name'),
            prop: 'name',
            flexGrow: 1
          },
          {
            name: gettext('Password'),
            prop: 'password',
            flexGrow: 1,
            cellTemplateName: 'template',
            cellTemplateConfig: '{{ "*" | repeat(5) }}'
          }
        ],
        actions: [
          {
            template: 'add',
            dialogConfig: {
              title: gettext('User'),
              fields: [
                {
                  type: 'textInput',
                  name: 'name',
                  value: '',
                  monospaced: true,
                  label: gettext('Name'),
                  validators: {
                    required: true
                  }
                },
                {
                  type: 'passwordInput',
                  name: 'password',
                  label: gettext('Password'),
                  value: '',
                  autocomplete: 'new-password',
                  validators: {
                    required: true
                  }
                }
              ]
            }
          },
          {
            template: 'delete'
          }
        ],
        valueType: 'object',
        value: []
      },
      {
        type: 'checkbox',
        name: 'usechroot',
        label: gettext('Use chroot'),
        hint: gettext(
          'If this option is set, the daemon will chroot to the shared folder path before starting the file transfer with the client. Then it is not possible to map users and groups by name and the daemon is not being able to follow symbolic links that are either absolute or outside of the new root path.'
        ),
        value: true
      },
      {
        type: 'checkbox',
        name: 'authusers',
        label: gettext('Authenticate users'),
        hint: gettext(
          'If set then the client will be challenged to supply a username and password to connect to the module.'
        ),
        value: false
      },
      {
        type: 'checkbox',
        name: 'readonly',
        label: gettext('Read-only'),
        hint: gettext('If this option is set, then any attempted uploads will fail.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'writeonly',
        label: gettext('Write-only'),
        hint: gettext('If this option is set, then any attempted downloads will fail.'),
        value: false
      },
      {
        type: 'checkbox',
        name: 'list',
        label: gettext('List module'),
        hint: gettext(
          'This option determines if this module should be listed when the client asks for a listing of available modules.'
        ),
        value: true
      },
      {
        type: 'numberInput',
        name: 'maxconnections',
        label: gettext('Max. connections'),
        hint: gettext(
          'This option specifies the maximum number of simultaneous connections. 0 means no limit.'
        ),
        value: 0,
        validators: {
          min: 0,
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'hostsallow',
        label: gettext('Hosts allow'),
        value: '',
        hint: gettext(
          'This option is a comma, space, or tab delimited set of hosts which are permitted to access this module. You can specify the hosts by name or IP number. Leave this field empty to use default settings.'
        )
      },
      {
        type: 'textInput',
        name: 'hostsdeny',
        label: gettext('Hosts deny'),
        value: '',
        hint: gettext(
          'This option is a comma, space, or tab delimited set of host which are NOT permitted to access this module. Where the lists conflict, the allow list takes precedence. In the event that it is necessary to deny all by default, use the keyword ALL (or the netmask 0.0.0.0/0) and then explicitly specify to the hosts allow parameter those hosts that should be permitted access. Leave this field empty to use default settings.'
        )
      },
      {
        type: 'textInput',
        name: 'extraoptions',
        label: gettext('Extra options'),
        hint: gettext(
          'Please check the <a href="http://www.samba.org/ftp/rsync/rsyncd.conf.html" target="_blank">manual page</a> for more details.'
        ),
        value: ''
      },
      {
        type: 'textInput',
        name: 'comment',
        label: gettext('Comment'),
        value: ''
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'url',
          url: '/services/rsync/server/modules'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/services/rsync/server/modules'
        }
      }
    ]
  };
}
