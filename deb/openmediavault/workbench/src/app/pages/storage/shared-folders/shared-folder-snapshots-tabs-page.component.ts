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
export class SharedFolderSnapshotsTabsPageComponent {
  public config: TabsPageConfig = {
    tabs: [
      {
        label: gettext('Snapshots'),
        type: 'datatable',
        config: {
          stateId: '7fcc8590-a2e3-11ed-ac0f-238d9ec75eda',
          autoReload: false,
          limit: 0,
          hasFooter: true,
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
              template: 'create',
              execute: {
                type: 'request',
                request: {
                  service: 'ShareMgmt',
                  method: 'createSnapshot',
                  params: {
                    uuid: '{{ _routeParams.uuid }}'
                  },
                  progressMessage: gettext('Please wait, a snapshot will be created ...'),
                  successNotification: gettext(
                    "The snapshot '{{ _response.name }}' has been created for the shared folder '{{ _response.sharedfolder }}'."
                  )
                }
              }
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
                  progressMessage: gettext('Please wait, a shared folder will be created ...'),
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
        }
      },
      {
        label: gettext('Scheduled Tasks'),
        type: 'datatable',
        config: {
          stateId: '5762ee82-ad1f-11ed-98df-0b261daceb5d',
          autoReload: false,
          limit: 0,
          hasFooter: true,
          hasSearchField: false,
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
                method: 'enumerateScheduledSnapshotTasks',
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
                  text: gettext('Hourly'),
                  execute: {
                    type: 'request',
                    request: {
                      service: 'ShareMgmt',
                      method: 'createScheduledSnapshotTask',
                      params: {
                        uuid: '{{ _routeParams.uuid }}',
                        execution: 'hourly'
                      },
                      successNotification: gettext(
                        'A scheduled task to create a snapshot has been created.'
                      )
                    }
                  }
                },
                {
                  text: gettext('Daily'),
                  execute: {
                    type: 'request',
                    request: {
                      service: 'ShareMgmt',
                      method: 'createScheduledSnapshotTask',
                      params: {
                        uuid: '{{ _routeParams.uuid }}',
                        execution: 'daily'
                      },
                      successNotification: gettext(
                        'A scheduled task to create a snapshot has been created.'
                      )
                    }
                  }
                },
                {
                  text: gettext('Weekly'),
                  execute: {
                    type: 'request',
                    request: {
                      service: 'ShareMgmt',
                      method: 'createScheduledSnapshotTask',
                      params: {
                        uuid: '{{ _routeParams.uuid }}',
                        execution: 'weekly'
                      },
                      successNotification: gettext(
                        'A scheduled task to create a snapshot has been created.'
                      )
                    }
                  }
                },
                {
                  text: gettext('Monthly'),
                  execute: {
                    type: 'request',
                    request: {
                      service: 'ShareMgmt',
                      method: 'createScheduledSnapshotTask',
                      params: {
                        uuid: '{{ _routeParams.uuid }}',
                        execution: 'monthly'
                      },
                      successNotification: gettext(
                        'A scheduled task to create a snapshot has been created.'
                      )
                    }
                  }
                },
                {
                  text: gettext('Yearly'),
                  execute: {
                    type: 'request',
                    request: {
                      service: 'ShareMgmt',
                      method: 'createScheduledSnapshotTask',
                      params: {
                        uuid: '{{ _routeParams.uuid }}',
                        execution: 'yearly'
                      },
                      successNotification: gettext(
                        'A scheduled task to create a snapshot has been created.'
                      )
                    }
                  }
                }
              ]
            },
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
      }
    ]
  };
}
