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
import { Component, OnInit, ViewChild } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { EMPTY } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { FormPageComponent } from '~/app/core/components/limn-ui/form-page/form-page.component';
import {
  FormPageButtonConfig,
  FormPageConfig
} from '~/app/core/components/limn-ui/models/form-page-config.type';
import { translate } from '~/app/i18n.helper';
import { RpcObjectResponse } from '~/app/shared/models/rpc.model';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  template: '<omv-limn-form-page [config]="this.config"></omv-limn-form-page>'
})
export class SharedFolderAclFormPageComponent implements OnInit {
  @BlockUI()
  blockUI: NgBlockUI;

  @ViewChild(FormPageComponent, { static: true })
  private formPage: FormPageComponent;

  public config: FormPageConfig = {
    fields: [
      {
        type: 'confObjUuid'
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
            cellTemplateName: 'buttonToogle',
            cellTemplateConfig: [
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

  constructor(private rpcService: RpcService) {}

  ngOnInit(): void {
    const self = this.formPage;
    self.editing = true;
    self.loadData = () => this.loadData('/');
    self.afterViewInitEvent.subscribe(() => {
      const control = self.form.formGroup.get('file');
      control.valueChanges.subscribe((value) => this.loadData(value));
    });
  }

  onSubmit(buttonConfig: FormPageButtonConfig, values: Record<string, any>) {
    this.blockUI.start(translate(gettext('Please wait, updating access control lists ...')));
    // Process RPC parameters.
    const perms = _.map(
      _.filter(values.perms, (o) => !_.isNull(o.perms)),
      (o) => {
        o.perms = _.parseInt(o.perms);
        return o;
      }
    );
    const users = _.filter(perms, ['type', 'user']);
    const groups = _.filter(perms, ['type', 'group']);
    const rpcParams = _.merge({ users, groups }, _.omit(values, 'perms'));
    this.rpcService
      .request('ShareMgmt', 'setFileACL', rpcParams)
      .pipe(
        finalize(() => {
          this.blockUI.stop();
        })
      )
      .subscribe();
  }

  protected loadData(file) {
    const self = this.formPage;
    const uuid = _.get(self.routeParams, 'uuid');
    self.loading = true;
    this.rpcService
      .request('ShareMgmt', 'getFileACL', {
        uuid,
        file
      })
      .pipe(
        catchError((error) => {
          self.error = error;
          return EMPTY;
        }),
        finalize(() => {
          self.loading = false;
        })
      )
      .subscribe((res: RpcObjectResponse) => {
        _.map(res.acl.users, (user: Record<string, any>) => _.set(user, 'type', 'user'));
        _.map(res.acl.groups, (group: Record<string, any>) => _.set(group, 'type', 'group'));
        self.onLoadData({
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
