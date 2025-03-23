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
export class SmbShareDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '14ee24b4-330a-11ea-b3fb-eb6c148bc874',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    rowEnumFmt: '{{ sharedfoldername }}',
    columns: [
      {
        name: gettext('Enabled'),
        prop: 'enable',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      { name: gettext('Shared folder'), prop: 'sharedfoldername', flexGrow: 1, sortable: true },
      {
        name: gettext('Comment'),
        prop: 'comment',
        cellTemplateName: 'text',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Public'),
        prop: 'guest',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'chip',
        cellTemplateConfig: {
          map: {
            no: { value: gettext('No') },
            allow: { value: gettext('Guests allowed') },
            only: { value: gettext('Guests only') }
          }
        }
      },
      {
        name: gettext('Read-only'),
        prop: 'readonly',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      {
        name: gettext('Browseable'),
        prop: 'browseable',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
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
        service: 'SMB',
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
          url: '/services/smb/shares/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/services/smb/shares/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'SMB',
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
