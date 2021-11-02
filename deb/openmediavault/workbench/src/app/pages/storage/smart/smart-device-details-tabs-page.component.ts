/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';

import { TabsPageConfig } from '~/app/core/components/intuition/models/tabs-page-config.type';

@Component({
  template: '<omv-intuition-tabs-page [config]="this.config"></omv-intuition-tabs-page>'
})
export class SmartDeviceDetailsTabsPageComponent {
  public config: TabsPageConfig = {
    tabs: [
      {
        label: gettext('Device Information'),
        type: 'form',
        config: {
          request: {
            service: 'Smart',
            get: {
              method: 'getInformation',
              params: {
                devicefile: '{{ _routeParams.devicefile }}'
              }
            }
          },
          fields: [
            {
              type: 'textInput',
              name: 'devicefile',
              label: gettext('Device'),
              disabled: true
            },
            {
              type: 'textInput',
              name: 'devicemodel',
              label: gettext('Model'),
              disabled: true
            },
            {
              type: 'textInput',
              name: 'serialnumber',
              label: gettext('Serial number'),
              disabled: true
            },
            {
              type: 'textInput',
              name: 'wwn',
              label: gettext('WWN'),
              disabled: true
            },
            {
              type: 'textInput',
              name: 'firmwareversion',
              label: gettext('Firmware version'),
              disabled: true
            }
          ],
          buttons: [
            {
              template: 'back',
              execute: {
                type: 'url',
                url: '/storage/smart/devices'
              }
            }
          ]
        }
      },
      {
        label: gettext('Attributes'),
        type: 'datatable',
        config: {
          autoReload: false,
          stateId: '7a0f3b78-4dc0-11ea-a8af-fb0e090c7ceb',
          hasFooter: false,
          sorters: [
            {
              dir: 'asc',
              prop: 'id'
            }
          ],
          store: {
            proxy: {
              service: 'Smart',
              get: {
                method: 'getAttributes',
                params: {
                  devicefile: '{{ _routeParams.devicefile }}'
                }
              }
            }
          },
          columns: [
            {
              name: gettext('ID'),
              prop: 'id',
              flexGrow: 1
            },
            {
              name: gettext('Name'),
              prop: 'attrname',
              flexGrow: 1
            },
            {
              name: gettext('Flags'),
              prop: 'flags',
              flexGrow: 1
            },
            {
              name: gettext('Value'),
              prop: 'value',
              flexGrow: 1
            },
            {
              name: gettext('Worst'),
              prop: 'worst',
              flexGrow: 1
            },
            {
              name: gettext('Threshold'),
              prop: 'threshold',
              flexGrow: 1
            },
            {
              name: gettext('When failed'),
              prop: 'whenfailed',
              flexGrow: 1
            },
            {
              name: gettext('Raw value'),
              prop: 'rawvalue',
              flexGrow: 1
            },
            {
              name: gettext('Type'),
              prop: 'prefailure',
              flexGrow: 1,
              cellTemplateName: 'chip',
              cellTemplateConfig: {
                map: {
                  true: { value: gettext('Prefail') },
                  false: { value: gettext('Old-age') }
                }
              }
            },
            {
              name: gettext('Status'),
              prop: 'assessment',
              flexGrow: 1,
              cellTemplateName: 'chip',
              cellTemplateConfig: {
                map: {
                  /* eslint-disable @typescript-eslint/naming-convention */
                  GOOD: { value: gettext('Good'), class: 'omv-chip-theme-success' },
                  BAD_STATUS: { value: gettext('Unknown') },
                  BAD_ATTRIBUTE_NOW: {
                    value: gettext('Bad'),
                    class: 'omv-chip-theme-error',
                    tooltip: gettext('Device is being used outside design parameters.')
                  },
                  BAD_ATTRIBUTE_IN_THE_PAST: {
                    value: gettext('Bad'),
                    class: 'omv-chip-theme-error',
                    tooltip: gettext('Device was used outside of design parameters in the past.')
                  },
                  BAD_SECTOR: {
                    value: gettext('Bad'),
                    class: 'omv-chip-theme-error',
                    tooltip: gettext('Device has a few bad sectors.')
                  },
                  BAD_SECTOR_MANY: {
                    value: gettext('Bad'),
                    class: 'omv-chip-theme-error',
                    tooltip: gettext('Device has many bad sectors.')
                  }
                }
              }
            }
          ],
          buttons: [
            {
              template: 'back',
              url: '/storage/smart/devices'
            }
          ]
        }
      },
      {
        label: gettext('Self-Test Logs'),
        type: 'datatable',
        config: {
          autoReload: false,
          stateId: '95a1f87e-4dc2-11ea-96be-0baa564da0f5',
          hasFooter: false,
          sorters: [
            {
              dir: 'asc',
              prop: 'id'
            }
          ],
          store: {
            proxy: {
              service: 'Smart',
              get: {
                method: 'getSelfTestLogs',
                params: {
                  devicefile: '{{ _routeParams.devicefile }}'
                }
              }
            }
          },
          columns: [
            {
              name: gettext('Num'),
              prop: 'num',
              flexGrow: 1
            },
            {
              name: gettext('Description'),
              prop: 'description',
              flexGrow: 1
            },
            {
              name: gettext('Status'),
              prop: 'status',
              flexGrow: 1
            },
            {
              name: gettext('Remaining'),
              prop: 'remaining',
              flexGrow: 1,
              cellTemplateName: 'template',
              cellTemplateConfig: '{{ remaining  }}%'
            },
            {
              name: gettext('Lifetime'),
              prop: 'lifetime',
              flexGrow: 1
            },
            {
              name: gettext('LBA of first error'),
              prop: 'lbaoffirsterror',
              flexGrow: 1
            }
          ],
          buttons: [
            {
              template: 'back',
              url: '/storage/smart/devices'
            }
          ]
        }
      },
      {
        label: gettext('Extended Information'),
        type: 'text',
        config: {
          hasReloadButton: true,
          request: {
            service: 'Smart',
            get: {
              method: 'getExtendedInformation',
              params: {
                devicefile: '{{ _routeParams.devicefile }}'
              }
            }
          },
          buttons: [
            {
              template: 'back',
              url: '/storage/smart/devices'
            }
          ]
        }
      }
    ]
  };
}
