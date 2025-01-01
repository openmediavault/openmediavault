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
import { Component, OnInit } from '@angular/core';
import { AbstractControl } from '@angular/forms';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { EMPTY } from 'rxjs';
import { catchError, distinctUntilChanged, finalize } from 'rxjs/operators';

import { FormValues } from '~/app/core/components/intuition/models/form.type';
import {
  FormPageButtonConfig,
  FormPageConfig
} from '~/app/core/components/intuition/models/form-page-config.type';
import { translate } from '~/app/i18n.helper';
import { BaseFormPageComponent } from '~/app/pages/base-page-component';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { RpcObjectResponse } from '~/app/shared/models/rpc.model';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class SharedFolderAclFormPageComponent extends BaseFormPageComponent implements OnInit {
  public config: FormPageConfig = {
    fields: [
      {
        type: 'sharedFolderSelect',
        name: 'uuid',
        label: gettext('Name'),
        disabled: true,
        hasCreateButton: false,
        value: '{{ _routeParams.uuid }}'
      },
      {
        type: 'folderBrowser',
        name: 'file',
        label: gettext('Relative path'),
        dirType: 'sharedfolder',
        dirRefIdField: 'uuid',
        readonly: true,
        value: '/'
      },
      {
        type: 'checkbox',
        name: 'replace',
        label: gettext('Replace'),
        hint: gettext('Replace all existing permissions.'),
        value: true
      },
      {
        type: 'checkbox',
        name: 'recursive',
        label: gettext('Recursive'),
        hint: gettext('Apply permissions to files and subfolders.'),
        value: false
      },
      {
        type: 'divider',
        title: gettext('File owner and group')
      },
      {
        type: 'container',
        fields: [
          {
            type: 'select',
            name: 'owner',
            label: gettext('Owner'),
            hint: gettext('Permissions of owner.'),
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
            value: 'root'
          },
          {
            type: 'select',
            name: 'userperms',
            label: gettext('Permissions'),
            store: {
              data: [
                [0, gettext('None')],
                [1, gettext('Execute only')],
                [2, gettext('Write only')],
                [3, gettext('Write/Execute')],
                [4, gettext('Read only')],
                [5, gettext('Read/Execute')],
                [6, gettext('Read/Write')],
                [7, gettext('Read/Write/Execute')]
              ]
            },
            value: 7
          }
        ]
      },
      {
        type: 'container',
        fields: [
          {
            type: 'select',
            name: 'group',
            label: gettext('Group'),
            hint: gettext('Permissions of group.'),
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
            value: 'users'
          },
          {
            type: 'select',
            name: 'groupperms',
            label: gettext('Permissions'),
            store: {
              data: [
                [0, gettext('None')],
                [1, gettext('Execute only')],
                [2, gettext('Write only')],
                [3, gettext('Write/Execute')],
                [4, gettext('Read only')],
                [5, gettext('Read/Execute')],
                [6, gettext('Read/Write')],
                [7, gettext('Read/Write/Execute')]
              ]
            },
            value: 7
          }
        ]
      },
      {
        type: 'select',
        name: 'otherperms',
        label: gettext('Others'),
        hint: gettext('Permissions of others (e.g. anonymous FTP users).'),
        store: {
          data: [
            [0, gettext('None')],
            [1, gettext('Execute only')],
            [2, gettext('Write only')],
            [3, gettext('Write/Execute')],
            [4, gettext('Read only')],
            [5, gettext('Read/Execute')],
            [6, gettext('Read/Write')],
            [7, gettext('Read/Write/Execute')]
          ]
        },
        value: 0
      },
      {
        type: 'divider',
        title: gettext('File access control lists')
      },
      {
        type: 'datatable',
        name: 'perms',
        label: gettext('User/Group permissions'),
        limit: 5,
        hasActionBar: true,
        hasSearchField: true,
        selectionType: 'none',
        columns: [
          { name: gettext('Name'), prop: 'name', flexGrow: 2, sortable: true },
          {
            name: gettext('Type'),
            prop: 'type',
            flexGrow: 1,
            sortable: true,
            cellTemplateName: 'chip',
            cellTemplateConfig: {
              map: {
                user: { value: gettext('User') },
                group: { value: gettext('Group') }
              }
            }
          },
          {
            name: gettext('System account'),
            prop: 'system',
            flexGrow: 1,
            cellTemplateName: 'checkIcon',
            sortable: true
          },
          {
            name: gettext('Permissions'),
            prop: 'perms',
            flexGrow: 3,
            sortable: true,
            cellTemplateName: 'buttonToggle',
            cellTemplateConfig: {
              buttons: [
                {
                  value: '7',
                  text: gettext('Read/Write')
                },
                {
                  value: '5',
                  text: gettext('Read-only')
                },
                {
                  value: '0',
                  text: gettext('No access')
                }
              ]
            }
          }
        ],
        actions: [
          {
            icon: 'mdi:transfer',
            tooltip: gettext('Copy permissions'),
            click: this.onCopyPermissions.bind(this)
          }
        ],
        sorters: [
          {
            dir: 'asc',
            prop: 'system'
          },
          {
            dir: 'asc',
            prop: 'name'
          }
        ],
        value: []
      }
    ],
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'click',
          click: this.onSubmit.bind(this)
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/storage/shared-folders'
        }
      }
    ]
  };

  constructor(
    private blockUiService: BlockUiService,
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private rpcService: RpcService
  ) {
    super();
  }

  ngOnInit(): void {
    const self = this.page;
    self.editing = true;
    self.loadData = () => this.loadData('/');
    self.afterViewInitEvent.subscribe(() => {
      const control: AbstractControl = self.form.formGroup.get('file');
      control?.valueChanges.pipe(distinctUntilChanged()).subscribe((value) => this.loadData(value));
    });
  }

  onCopyPermissions() {
    const uuid: string = _.get(this.page.routeParams, 'uuid');
    const values: FormValues = this.page.getFormValues();
    this.dialogService
      .open(ModalDialogComponent, {
        data: {
          template: 'confirmation',
          title: gettext('Copy permissions'),
          message: gettext('Do you really want to copy the permissions from the shared folder?')
        }
      })
      .afterClosed()
      .subscribe((choice: boolean) => {
        if (choice) {
          this.blockUiService.start(
            translate(gettext('Please wait, updating the permissions ...'))
          );
          this.rpcService
            .request('ShareMgmt', 'getPrivileges', { uuid })
            .pipe(
              finalize(() => {
                this.blockUiService.stop();
              })
            )
            .subscribe((privs: Record<string, any>) => {
              const perms = _.cloneDeep(values.perms);
              // Reset all permission by default.
              _.forEach(perms, (item) => {
                item.perms = null;
              });
              // Apply the permissions from the shared folder.
              _.forEach(privs, (priv) => {
                const a = _.find(perms, { name: priv.name, type: priv.type });
                if (!_.isUndefined(a)) {
                  a.perms = priv.perms;
                }
              });
              this.page.setFormValues({ perms }, false);
            });
        }
      });
  }

  onSubmit(buttonConfig: FormPageButtonConfig, values: Record<string, any>) {
    this.blockUiService.start(translate(gettext('Please wait, updating access control lists ...')));
    // Process RPC parameters.
    const perms = _.map(_.reject(values.perms, ['perms', null]), (obj) => {
      obj.perms = _.toInteger(obj.perms);
      return obj;
    });
    const users = _.filter(perms, ['type', 'user']);
    const groups = _.filter(perms, ['type', 'group']);
    const rpcParams = _.merge({ users, groups }, _.omit(values, 'perms'));
    this.rpcService
      .requestTask('ShareMgmt', 'setFileACL', rpcParams)
      .pipe(
        finalize(() => {
          this.blockUiService.stop();
        })
      )
      .subscribe(() => {
        this.page.markAsPristine();
        this.notificationService.show(
          NotificationType.success,
          _.get(this.page.routeConfig, 'data.notificationTitle')
        );
      });
  }

  protected loadData(file: string) {
    const uuid: string = _.get(this.page.routeParams, 'uuid');
    this.page.loading = true;
    this.rpcService
      .request('ShareMgmt', 'getFileACL', {
        uuid,
        file
      })
      .pipe(
        catchError((error) => {
          this.page.error = error;
          return EMPTY;
        }),
        finalize(() => {
          this.page.loading = false;
        })
      )
      .subscribe((res: RpcObjectResponse) => {
        _.map(res.acl.users, (user: Record<string, any>) => _.set(user, 'type', 'user'));
        _.map(res.acl.groups, (group: Record<string, any>) => _.set(group, 'type', 'group'));
        this.page.setFormValues({
          owner: res.owner,
          group: res.group,
          userperms: res.acl.user,
          groupperms: res.acl.group,
          otherperms: res.acl.other,
          perms: _.concat(res.acl.users, res.acl.groups)
        });
      });
  }
}
