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
export class SslCertificateDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '9fe3e818-1c32-11ea-bde4-5f0ce98b6927',
    columns: [
      {
        name: gettext('Valid from'),
        prop: 'validfrom',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'localeDateTime'
      },
      {
        name: gettext('Valid to'),
        prop: 'validto',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'localeDateTime'
      },
      {
        name: gettext('Fingerprint (SHA-1)'),
        prop: 'fingerprintsha1',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'copyToClipboard'
      },
      {
        name: gettext('Fingerprint (SHA-256)'),
        prop: 'fingerprintsha256',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'copyToClipboard'
      },
      {
        name: gettext('Referenced'),
        prop: '_used',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'checkIcon'
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
        prop: 'comment'
      }
    ],
    store: {
      proxy: {
        service: 'CertificateMgmt',
        get: {
          method: 'getList'
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
              url: '/system/certificate/ssl/create'
            }
          },
          {
            type: 'iconButton',
            text: gettext('Import'),
            icon: 'import',
            execute: {
              type: 'url',
              url: '/system/certificate/ssl/import'
            }
          }
        ]
      },
      {
        type: 'iconButton',
        icon: 'details',
        tooltip: gettext('Show details'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'url',
          url: '/system/certificate/ssl/detail/{{ _selected[0].uuid }}'
        }
      },
      {
        template: 'delete',
        enabledConstraints: {
          constraint: [
            // Disable button if a selected SSL certificate is in use.
            { operator: 'falsy', arg0: { prop: '_used' } }
          ]
        },
        execute: {
          type: 'request',
          request: {
            service: 'CertificateMgmt',
            method: 'delete',
            params: {
              uuid: '{{ uuid }}'
            }
          }
        }
      }
    ]
  };
}
