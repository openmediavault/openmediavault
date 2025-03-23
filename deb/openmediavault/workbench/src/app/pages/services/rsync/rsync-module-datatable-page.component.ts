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
export class RsyncModuleDatatablePageComponent {
  public config: DatatablePageConfig = {
    autoReload: false,
    stateId: '5b146d0b-2250-4ea2-a16d-bddb7e66724e',
    sorters: [
      {
        dir: 'asc',
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'Rsyncd',
        get: {
          method: 'getModuleList'
        }
      }
    },
    rowEnumFmt: '{{ sharedfoldername }}',
    columns: [
      {
        name: gettext('Enabled'),
        prop: 'enable',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
      },
      {
        name: gettext('Shared folder'),
        prop: 'sharedfoldername',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Name'),
        prop: 'name',
        cellTemplateName: 'text',
        flexGrow: 1,
        sortable: true
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
    actions: [
      {
        template: 'create',
        execute: {
          type: 'url',
          url: '/services/rsync/server/modules/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/services/rsync/server/modules/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'Rsyncd',
            method: 'deleteModule',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };
}
