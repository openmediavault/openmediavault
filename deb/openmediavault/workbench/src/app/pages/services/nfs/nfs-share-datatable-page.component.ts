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
export class NfsShareDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '4aa1756c-c761-403d-91fe-d67985cd8133',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    rowEnumFmt: '{{ sharedfoldername }}',
    columns: [
      { name: gettext('Shared folder'), prop: 'sharedfoldername', flexGrow: 1, sortable: true },
      { name: gettext('Client'), prop: 'client', flexGrow: 1, sortable: true },
      {
        name: gettext('Options'),
        prop: 'options',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'template',
        cellTemplateConfig:
          '{{ extraoptions | split(",") | union(options | split(",")) | uniq() | sort() | join(", ") }}'
      },
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
        prop: 'sharedfoldername'
      }
    ],
    store: {
      proxy: {
        service: 'NFS',
        get: {
          method: 'getShareList'
        }
      }
    },
    actions: [
      {
        template: 'create',
        execute: {
          type: 'url',
          url: '/services/nfs/shares/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/services/nfs/shares/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'NFS',
            method: 'deleteShare',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };
}
