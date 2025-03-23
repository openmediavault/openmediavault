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
export class FilesystemQuotaDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '3f8c62be-11ba-4d37-8683-cbe44c049955',
    autoReload: false,
    remoteSorting: false,
    remotePaging: false,
    hasSearchField: true,
    sorters: [
      {
        dir: 'asc',
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'Quota',
        get: {
          method: 'get',
          params: {
            uuid: '{{ _routeParams.uuid }}'
          }
        }
      }
    },
    columns: [
      { name: gettext('Name'), prop: 'name', flexGrow: 1, sortable: true },
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
        name: gettext('Used capacity'),
        prop: 'bused',
        flexGrow: 1,
        sortable: true
      },
      {
        name: gettext('Quota'),
        prop: 'bhardlimit',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'template',
        cellTemplateConfig: '{{ bhardlimit }} {{ bunit }}'
      }
    ],
    actions: [
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/storage/filesystems/quota/{{ _routeParams.uuid }}/edit/{{ _selected[0].type }}/{{ _selected[0].name }}'
        }
      }
    ]
  };
}
