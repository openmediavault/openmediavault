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
import dayjs from 'dayjs';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';

import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { Unsubscribe } from '~/app/decorators';
import { format, notAvailable } from '~/app/functions.helper';
import { BinaryUnitPipe } from '~/app/shared/pipes/binary-unit.pipe';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';

@Component({
  template: '<omv-intuition-datatable-page [config]="this.config"></omv-intuition-datatable-page>'
})
export class SystemInformationDatatablePageComponent {
  @Unsubscribe()
  private subscriptions = new Subscription();

  public config: DatatablePageConfig = {
    limit: 0,
    autoReload: false,
    hasActionBar: false,
    hasHeader: false,
    hasFooter: false,
    selectionType: 'none',
    columns: [
      {
        prop: 'name',
        flexGrow: 1,
        cellClass: 'omv-font-weight-bold',
        cellTemplateName: 'template',
        cellTemplateConfig: '{{ name | translate }}'
      },
      {
        prop: 'value',
        flexGrow: 3,
        cellTemplateName: 'shapeShifter'
      }
    ],
    store: {
      data: [],
      fields: ['name', 'value']
    }
  };

  constructor(
    private binaryUnitPipe: BinaryUnitPipe,
    private systemInformationService: SystemInformationService
  ) {
    this.subscriptions.add(
      this.systemInformationService.systemInfo$.subscribe((res: SystemInformation) => {
        const data = [];
        const rows = {
          hostname: {
            name: gettext('Hostname'),
            value: {
              type: 'text',
              value: _.get(res, 'hostname')
            }
          },
          version: {
            name: gettext('Version'),
            value: {
              type: 'text',
              value: notAvailable(_.get(res, 'version'))
            }
          },
          cpuModelName: {
            name: gettext('Processor'),
            value: {
              type: 'text',
              value: notAvailable(_.get(res, 'cpuModelName'))
            }
          },
          kernel: {
            name: gettext('Kernel'),
            value: {
              type: 'text',
              value: notAvailable(_.get(res, 'kernel'))
            }
          },
          time: {
            name: gettext('System Time'),
            value: {
              type: 'localeDateTime',
              value: _.get(res, 'ts')
            }
          },
          uptime: {
            name: gettext('Uptime'),
            value: {
              type: 'relativeTime',
              value: dayjs().unix() - _.get(res, 'uptime')
            }
          },
          loadAverage: {
            name: gettext('Load Average'),
            value: {
              type: 'text',
              value: format(
                '{{ loadAverage.1min | tofixed(2) }}, {{ loadAverage.5min | tofixed(2) }}, {{ loadAverage.15min | tofixed(2) }}',
                res
              )
            }
          },
          cpuUsage: {
            name: gettext('CPU Usage'),
            value: {
              type: 'progressBar',
              text: `${res.cpuUsage?.toFixed(1)}%`,
              value: res.cpuUsage?.toFixed(1)
            }
          },
          memUsed: {
            name: gettext('Memory Usage'),
            value: {
              type: 'progressBar',
              text: `${(res.memUtilization * 100).toFixed(1)}% of ${this.binaryUnitPipe.transform(
                res.memTotal
              )}`,
              value: (res.memUtilization * 100).toFixed(1)
            }
          }
        };
        _.forEach(rows, (value: Record<any, any>, key: string) => {
          if (_.has(res, key)) {
            data.push(value);
          }
        });
        this.config.store.data = data;
      })
    );
  }
}
