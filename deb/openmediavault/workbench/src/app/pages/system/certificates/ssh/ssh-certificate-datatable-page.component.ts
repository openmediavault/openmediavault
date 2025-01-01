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

import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';

@Component({
  template: '<omv-intuition-datatable-page [config]="this.config"></omv-intuition-datatable-page>'
})
export class SshCertificateDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '85efa226-1c32-11ea-8f7a-67b9a1e57494',
    columns: [
      {
        name: gettext('Tags'),
        prop: 'comment',
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          separator: ','
        },
        flexGrow: 1,
        sortable: true
      }
    ],
    sorters: [
      {
        dir: 'asc',
        prop: 'comment'
      }
    ],
    store: {
      proxy: {
        service: 'CertificateMgmt',
        get: {
          method: 'getSshList'
        }
      }
    },
    actions: [
      {
        type: 'menu',
        icon: 'add',
        tooltip: gettext('Create | Import'),
        actions: [
          {
            template: 'create',
            execute: {
              type: 'url',
              url: '/system/certificate/ssh/create'
            }
          },
          {
            type: 'iconButton',
            text: gettext('Import'),
            icon: 'import',
            execute: {
              type: 'url',
              url: '/system/certificate/ssh/import'
            }
          }
        ]
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/system/certificate/ssh/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        type: 'iconButton',
        icon: 'copy',
        tooltip: gettext('Copy'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'formDialog',
          formDialog: {
            title: gettext('Copy public SSH key'),
            fields: [
              {
                type: 'hint',
                hintType: 'info',
                text: gettext(
                  'Installs the public SSH key on a remote system as an authorized key. Make sure password authentication is enabled on that remote system.'
                )
              },
              {
                type: 'hidden',
                name: 'uuid',
                value: '{{ _selected[0].uuid }}'
              },
              {
                type: 'textInput',
                name: 'hostname',
                label: gettext('Hostname'),
                hint: gettext('The hostname of the remote system.'),
                value: '',
                validators: {
                  required: true
                }
              },
              {
                type: 'numberInput',
                name: 'port',
                label: gettext('Port'),
                hint: gettext('The port on which SSH is running on the remote system.'),
                value: 22,
                validators: {
                  required: true,
                  patternType: 'port'
                }
              },
              {
                type: 'textInput',
                name: 'username',
                label: gettext('User name'),
                value: '',
                autocomplete: 'username',
                validators: {
                  required: true
                }
              },
              {
                type: 'passwordInput',
                name: 'password',
                label: gettext('Password'),
                value: '',
                autocomplete: 'new-password'
              }
            ],
            buttons: {
              submit: {
                text: gettext('Copy'),
                execute: {
                  type: 'request',
                  request: {
                    service: 'CertificateMgmt',
                    method: 'copySshId',
                    progressMessage: gettext(
                      'Please wait, installing public SSH key on remote system ...'
                    ),
                    successNotification: gettext('Copied SSH certificate to {{ hostname }}.')
                  }
                }
              }
            }
          }
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'CertificateMgmt',
            method: 'deleteSsh',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };
}
