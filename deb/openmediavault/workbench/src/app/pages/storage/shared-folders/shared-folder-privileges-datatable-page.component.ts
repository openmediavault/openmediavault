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
import { Component, ViewChild } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { DatatablePageComponent } from '~/app/core/components/intuition/datatable-page/datatable-page.component';
import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { format } from '~/app/functions.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  template:
    '<omv-intuition-datatable-page #page [config]="this.config"></omv-intuition-datatable-page>'
})
export class SharedFolderPrivilegesDatatablePageComponent {
  @ViewChild('page', { static: true })
  page: DatatablePageComponent;

  public config: DatatablePageConfig = {
    stateId: '99f40468-8309-11ea-834f-cbe87c99180b',
    autoReload: false,
    limit: 0,
    hasFooter: false,
    hasSearchField: true,
    icon: 'tip',
    subTitle: gettext(
      // eslint-disable-next-line max-len
      'These settings are used by the services to configure the user and group access rights. Please note that these settings have no effect on file system permissions.'
    ),
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
        dir: 'desc',
        prop: 'type'
      },
      {
        dir: 'asc',
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'ShareMgmt',
        get: {
          method: 'getPrivileges',
          params: {
            uuid: '{{ _routeParams.uuid }}'
          }
        }
      }
    },
    actions: [
      {
        type: 'iconButton',
        icon: 'mdi:transfer',
        tooltip: gettext('Copy privileges'),
        execute: {
          type: 'formDialog',
          formDialog: {
            title: gettext('Copy privileges'),
            fields: [
              {
                type: 'sharedFolderSelect',
                name: 'src',
                store: {
                  filters: [
                    { operator: 'ne', arg0: { prop: 'uuid' }, arg1: '{{ _routeParams.uuid }}' }
                  ]
                },
                label: gettext('Source'),
                hint: gettext('The shared folder from which the privileges are copied.'),
                validators: {
                  required: true
                }
              },
              {
                type: 'hidden',
                name: 'dst',
                value: '{{ _routeParams.uuid }}'
              }
            ],
            buttons: {
              submit: {
                text: gettext('Transfer'),
                execute: {
                  type: 'request',
                  request: {
                    service: 'ShareMgmt',
                    method: 'copyPrivileges',
                    successNotification: gettext('Shared folder permissions have been transferred.')
                  }
                }
              }
            }
          }
        }
      }
    ],
    buttons: [
      {
        text: gettext('Save'),
        class: 'omv-background-color-theme-primary',
        click: this.onSave.bind(this)
      },
      {
        text: gettext('Cancel'),
        url: '/storage/shared-folders'
      }
    ]
  };

  constructor(
    private router: Router,
    private rpcService: RpcService,
    private notificationService: NotificationService
  ) {}

  onSave() {
    const privileges = [];
    _.forEach(this.page.table.data, (row) => {
      const privilege = {
        type: _.get(row, 'type'),
        name: _.get(row, 'name'),
        perms: _.toInteger(_.get(row, 'perms'))
      };
      if (_.isNull(privilege.perms)) {
        privilege.perms = -1;
      }
      privileges.push(privilege);
    });
    this.rpcService
      .request('ShareMgmt', 'setPrivileges', {
        uuid: _.get(this.page.routeParams, 'uuid'),
        privileges
      })
      .subscribe(() => {
        this.notificationService.show(
          NotificationType.success,
          format(_.get(this.page.routeConfig, 'data.notificationTitle'), this.page.routeParams)
        );
        this.router.navigate(['/storage/shared-folders']);
      });
  }
}
