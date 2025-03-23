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

import { TabsPageConfig } from '~/app/core/components/intuition/models/tabs-page-config.type';

@Component({
  template: '<omv-intuition-tabs-page [config]="this.config"></omv-intuition-tabs-page>'
})
export class SharedFolderAllSnapshotsTabsPageComponent {
  public config: TabsPageConfig = {
    tabs: [
      {
        label: gettext('Snapshots'),
        type: 'datatable',
        config: {
          stateId: 'beae0ef8-bfff-11ed-8ff5-17628e8426cf',
          autoReload: false,
          limit: 0,
          hasFooter: true,
          hasSearchField: true,
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
                method: 'enumerateAllSnapshots'
              }
            }
          },
          actions: [
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
                    uuid: '{{ sharedfolderref }}',
                    id: '{{ id }}'
                  },
                  successNotification: gettext(
                    'The shared folder {{ _selected[0].name }} was successfully created.'
                  )
                }
              }
            },
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
                    uuid: '{{ sharedfolderref }}',
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
        }
      },
      {
        label: gettext('Scheduled Tasks'),
        type: 'datatable',
        config: {
          stateId: 'd1f00a2c-c002-11ed-9171-6f9f7fbdd288',
          autoReload: false,
          limit: 0,
          hasFooter: true,
          hasSearchField: true,
          selectionType: 'multi',
          columns: [
            {
              name: gettext('Enabled'),
              prop: 'enable',
              flexGrow: 1,
              sortable: true,
              cellTemplateName: 'checkIcon'
            },
            {
              name: gettext('Scheduling'),
              prop: '',
              flexGrow: 1,
              cellTemplateName: 'template',
              cellTemplateConfig:
                '{% if execution == "exactly" %}' +
                '{% set _minute = minute %}' +
                '{% set _hour = hour %}' +
                '{% set _dayofmonth = dayofmonth %}' +
                '{% if everynminute %}{% set _minute %}*/{{ minute }}{% endset %}{% endif %}' +
                '{% if everynhour %}{% set _hour %}*/{{ hour }}{% endset %}{% endif %}' +
                '{% if everyndayofmonth %}{% set _dayofmonth %}*/{{ dayofmonth }}{% endset %}{% endif %}' +
                '{{ [_minute, _hour, _dayofmonth, month, dayofweek] | join(" ") | cron2human }}' +
                '{% else %}' +
                '{{ execution | capitalize | translate }}' +
                '{% endif %}'
            },
            {
              name: gettext('Command'),
              prop: 'command',
              cellTemplateName: 'text',
              flexGrow: 1,
              sortable: true,
              hidden: true
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
              prop: 'enable'
            }
          ],
          store: {
            proxy: {
              service: 'ShareMgmt',
              get: {
                method: 'enumerateAllScheduledSnapshotTasks'
              }
            }
          },
          actions: [
            {
              template: 'edit',
              execute: {
                type: 'url',
                url: '/system/cron/edit/{{ _selected[0].uuid }}'
              }
            },
            {
              template: 'delete',
              execute: {
                type: 'request',
                request: {
                  service: 'Cron',
                  method: 'delete',
                  params: {
                    uuid: '{{ uuid }}'
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
        }
      },
      {
        label: gettext('Settings'),
        type: 'form',
        config: {
          request: {
            service: 'ShareMgmt',
            get: {
              method: 'getSnapshotLifecycle'
            },
            post: {
              method: 'setSnapshotLifecycle'
            }
          },
          fields: [
            {
              type: 'checkbox',
              name: 'enable',
              label: gettext('Enable automatic snapshot clean up.'),
              value: false
            },
            {
              type: 'numberInput',
              name: 'retentionperiod',
              label: gettext('Retention period'),
              hint: gettext(
                'The retention period (in seconds) before a snapshot can be deleted. This ensures that a snapshot that has just been created cannot be deleted again immediately.'
              ),
              value: 1800,
              validators: {
                min: 0,
                max: 65535,
                patternType: 'integer'
              }
            },
            {
              type: 'numberInput',
              name: 'limitcustom',
              label: gettext('Limit custom'),
              hint: gettext('The number of custom snapshots to keep.'),
              value: 10,
              validators: {
                min: 0,
                max: 1024,
                patternType: 'integer'
              }
            },
            {
              type: 'numberInput',
              name: 'limithourly',
              label: gettext('Limit hourly'),
              hint: gettext('The number of hourly snapshots to keep.'),
              value: 24,
              validators: {
                min: 0,
                max: 1024,
                patternType: 'integer'
              }
            },
            {
              type: 'numberInput',
              name: 'limitdaily',
              label: gettext('Limit daily'),
              hint: gettext('The number of daily snapshots to keep.'),
              value: 7,
              validators: {
                min: 0,
                max: 1024,
                patternType: 'integer'
              }
            },
            {
              type: 'numberInput',
              name: 'limitweekly',
              label: gettext('Limit weekly'),
              hint: gettext('The number of weekly snapshots to keep.'),
              value: 4,
              validators: {
                min: 0,
                max: 1024,
                patternType: 'integer'
              }
            },
            {
              type: 'numberInput',
              name: 'limitmonthly',
              label: gettext('Limit monthly'),
              hint: gettext('The number of monthly snapshots to keep.'),
              value: 12,
              validators: {
                min: 0,
                max: 1024,
                patternType: 'integer'
              }
            },
            {
              type: 'numberInput',
              name: 'limityearly',
              label: gettext('Limit yearly'),
              hint: gettext('The number of yearly snapshots to keep.'),
              value: 1,
              validators: {
                min: 0,
                max: 1024,
                patternType: 'integer'
              }
            }
          ],
          buttons: [
            {
              template: 'submit'
            },
            {
              template: 'back',
              execute: {
                type: 'url',
                url: '/storage/shared-folders'
              }
            }
          ]
        }
      }
    ]
  };
}
