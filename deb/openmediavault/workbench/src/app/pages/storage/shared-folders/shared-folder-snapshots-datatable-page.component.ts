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
export class SharedFolderSnapshotsDatatablePageComponent {
  public config: DatatablePageConfig = {
    stateId: '7fcc8590-a2e3-11ed-ac0f-238d9ec75eda',
    autoReload: false,
    limit: 0,
    hasFooter: false,
    hasSearchField: false,
    selectionType: 'multi',
    columns: [
      {
        name: gettext('ID'),
        prop: 'id',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Name'),
        prop: 'name',
        flexGrow: 2,
        sortable: true
      },
      {
        name: gettext('Creation Time'),
        prop: 'otimets',
        flexGrow: 1,
        sortable: true,
        cellTemplateName: 'localeDateTime'
      },
      {
        name: gettext('UUID'),
        prop: 'uuid',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'copyToClipboard'
      },
      {
        name: gettext('Parent UUID'),
        prop: 'parent_uuid',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'copyToClipboard'
      },
      {
        name: gettext('Relative Path'),
        prop: 'path',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'copyToClipboard'
      },
      {
        name: gettext('Absolute Path'),
        prop: 'abspath',
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
      }
    ],
    sorters: [
      {
        dir: 'asc',
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'ShareMgmt',
        get: {
          method: 'enumerateSnapshots',
          params: {
            uuid: '{{ _routeParams.uuid }}'
          }
        }
      }
    },
    actions: [
      {
        type: 'menu',
        icon: 'add',
        tooltip: gettext('Create'),
        actions: [
          {
            text: gettext('Now'),
            execute: {
              type: 'request',
              request: {
                service: 'ShareMgmt',
                method: 'createSnapshot',
                params: {
                  uuid: '{{ _routeParams.uuid }}'
                },
                successNotification: gettext(
                  "The snapshot '{{ _response.name }}' has been created for the shared folder '{{ _response.sharedfolder }}'."
                )
              }
            }
          },
          {
            text: gettext('Hourly'),
            execute: {
              type: 'request',
              request: {
                service: 'ShareMgmt',
                method: 'createScheduledSnapshot',
                params: {
                  uuid: '{{ _routeParams.uuid }}',
                  execution: 'hourly'
                },
                successNotification: gettext('A scheduled snapshot has been created.'),
                successUrl: '/system/cron/edit/{{ _response.uuid }}'
              }
            }
          },
          {
            text: gettext('Daily'),
            execute: {
              type: 'request',
              request: {
                service: 'ShareMgmt',
                method: 'createScheduledSnapshot',
                params: {
                  uuid: '{{ _routeParams.uuid }}',
                  execution: 'daily'
                },
                successNotification: gettext('A scheduled snapshot has been created.'),
                successUrl: '/system/cron/edit/{{ _response.uuid }}'
              }
            }
          },
          {
            text: gettext('Weekly'),
            execute: {
              type: 'request',
              request: {
                service: 'ShareMgmt',
                method: 'createScheduledSnapshot',
                params: {
                  uuid: '{{ _routeParams.uuid }}',
                  execution: 'weekly'
                },
                successNotification: gettext('A scheduled snapshot has been created.'),
                successUrl: '/system/cron/edit/{{ _response.uuid }}'
              }
            }
          },
          {
            text: gettext('Monthly'),
            execute: {
              type: 'request',
              request: {
                service: 'ShareMgmt',
                method: 'createScheduledSnapshot',
                params: {
                  uuid: '{{ _routeParams.uuid }}',
                  execution: 'monthly'
                },
                successNotification: gettext('A scheduled snapshot has been created.'),
                successUrl: '/system/cron/edit/{{ _response.uuid }}'
              }
            }
          }
        ]
      },
      {
        type: 'iconButton',
        icon: 'share',
        tooltip: gettext('Share'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1,
          constraint: [{ operator: 'falsy', arg0: { prop: '_used' } }]
        },
        execute: {
          type: 'request',
          request: {
            service: 'ShareMgmt',
            method: 'fromSnapshot',
            params: {
              uuid: '{{ _routeParams.uuid }}',
              id: '{{ id }}'
            },
            successNotification: gettext(
              'The shared folder {{ _selected[0].name }} was successfully created.'
            )
          }
        }
      },
      // {
      //   type: 'iconButton',
      //   icon: 'restore',
      //   tooltip: gettext('Restore'),
      //   enabledConstraints: {
      //     minSelected: 1,
      //     maxSelected: 1
      //   },
      //   execute: {
      //     type: 'request',
      //     request: {
      //       service: 'ShareMgmt',
      //       method: 'restoreSnapshot',
      //       params: {
      //         uuid: '{{ _routeParams.uuid }}',
      //         id: '{{ id }}'
      //       }
      //     }
      //   }
      // },
      {
        template: 'delete',
        enabledConstraints: {
          constraint: [
            // Disable button if the snapshot is in use.
            { operator: 'falsy', arg0: { prop: '_used' } }
          ]
        },
        execute: {
          type: 'request',
          request: {
            service: 'ShareMgmt',
            method: 'deleteSnapshot',
            params: {
              uuid: '{{ _routeParams.uuid }}',
              id: '{{ id }}'
            }
          }
        }
      }
    ],
    buttons: [
      {
        template: 'back',
        url: '/storage/shared-folders'
      }
    ]
  };
}
