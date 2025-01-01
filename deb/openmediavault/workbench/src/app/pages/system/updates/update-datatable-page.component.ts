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
export class UpdateDatatablePageComponent {
  public config: DatatablePageConfig = {
    columns: [
      {
        name: gettext('Package Information'),
        prop: '',
        flexGrow: 1,
        cellTemplateName: 'template',
        cellTemplateConfig:
          '<div>' +
          '  <div class="omv-display-flex omv-flex-column">' +
          '    <div class="omv-font-weight-title omv-font-size-title">{{ name }} {{ version }}</div>' +
          '    <div class="omv-font-size-subheading-2">{{ summary }}</div>' +
          '  </div>' +
          '  <br>' +
          '{% if extendeddescription %}  <span>{{ extendeddescription }}</span><br><br>{% endif %}' +
          '{% if maintainer %}  <span>{{ "Maintainer" | translate }}: {{ maintainer }}</span><br>{% endif %}' +
          // eslint-disable-next-line max-len
          '{% if homepage %}  <span>{{ "Homepage" | translate }}: <a href="{{ homepage }}">{{ homepage }}</a></span><br>{% endif %}' +
          '{% if repository %}  <span>{{ "Repository" | translate }}: {{ repository }}</span><br>{% endif %}' +
          '{% if size %}  <span>{{ "Size" | translate }}: {{ size | binaryunit }}</span><br>{% endif %}' +
          '</div>'
      },
      { name: gettext('Name'), prop: 'name', flexGrow: 1, sortable: true, hidden: true },
      { name: gettext('Version'), prop: 'version', flexGrow: 1, sortable: true, hidden: true },
      {
        name: gettext('Repository'),
        prop: 'repository',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      { name: gettext('Abstract'), prop: 'abstract', flexGrow: 1, sortable: true, hidden: true },
      {
        name: gettext('Description'),
        prop: 'extendeddescription',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      {
        name: gettext('Size'),
        prop: 'size',
        flexGrow: 1,
        sortable: true,
        hidden: true,
        cellTemplateName: 'binaryUnit'
      },
      {
        name: gettext('Maintainer'),
        prop: 'maintainer',
        flexGrow: 1,
        sortable: true,
        hidden: true
      },
      { name: gettext('Homepage'), prop: 'homepage', flexGrow: 1, sortable: true, hidden: true }
    ],
    selectionType: 'single',
    limit: 0,
    sorters: [
      {
        dir: 'asc',
        prop: 'name'
      }
    ],
    store: {
      proxy: {
        service: 'Apt',
        get: {
          method: 'enumerateUpgraded'
        }
      }
    },
    stateId: '6b78ea5a-1fda-11ea-a62d-23f52be9bf23',
    actions: [
      {
        type: 'iconButton',
        icon: 'search',
        tooltip: gettext('Check for new updates'),
        execute: {
          type: 'request',
          request: {
            service: 'Apt',
            method: 'update',
            task: true,
            progressMessage: gettext(
              'The repository will be checked for new, removed or upgraded software packages.'
            )
          }
        }
      },
      {
        type: 'iconButton',
        icon: 'mdi:download',
        tooltip: gettext('Install updates'),
        hasBadge: 'numDataItems',
        enabledConstraints: {
          hasData: true
        },
        confirmationDialogConfig: {
          template: 'confirmation-critical',
          message: gettext('All packages will be upgraded. Do you want to proceed?')
        },
        execute: {
          type: 'taskDialog',
          taskDialog: {
            config: {
              title: gettext('Upgrade system'),
              startOnInit: true,
              buttons: {
                start: {
                  hidden: true
                },
                stop: {
                  hidden: true
                },
                close: {
                  dialogResult: true
                }
              },
              request: {
                service: 'Apt',
                method: 'upgrade',
                maxRetries: 5
              }
            },
            successUrl: '/reload'
          }
        }
      },
      {
        type: 'iconButton',
        icon: 'details',
        tooltip: gettext('Show changelog'),
        enabledConstraints: {
          minSelected: 1,
          maxSelected: 1
        },
        execute: {
          type: 'taskDialog',
          taskDialog: {
            config: {
              title: gettext('Changelog'),
              autoScroll: false,
              startOnInit: true,
              showCompletion: false,
              buttons: {
                start: {
                  hidden: true
                },
                stop: {
                  hidden: true
                }
              },
              request: {
                service: 'Apt',
                method: 'getChangeLog',
                params: {
                  filename: '{{ _selected[0].filename }}'
                }
              }
            }
          }
        }
      }
    ]
  };
}
