/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2023 Volker Theile
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
export class FtpBanRuleDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '5cb9dfa8-36fd-11ea-99ec-5ba8c267c020',
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    columns: [
      { name: gettext('Event'), prop: 'event', flexGrow: 1, sortable: true },
      { name: gettext('Occurrence'), prop: 'occurrence', flexGrow: 1, sortable: true },
      { name: gettext('Time interval'), prop: 'timeinterval', flexGrow: 1, sortable: true },
      { name: gettext('Expire'), prop: 'expire', flexGrow: 1, sortable: true }
    ],
    sorters: [
      {
        dir: 'asc',
        prop: 'event'
      }
    ],
    store: {
      proxy: {
        service: 'FTP',
        get: {
          method: 'getModBanRuleList'
        }
      }
    },
    actions: [
      {
        template: 'create',
        execute: {
          type: 'url',
          url: '/services/ftp/ban-rules/create'
        }
      },
      {
        template: 'edit',
        execute: {
          type: 'url',
          url: '/services/ftp/ban-rules/edit/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        execute: {
          type: 'request',
          request: {
            service: 'FTP',
            method: 'deleteModBanRule',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };
}
