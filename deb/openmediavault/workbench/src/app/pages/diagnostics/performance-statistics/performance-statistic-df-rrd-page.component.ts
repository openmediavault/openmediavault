/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2024 Volker Theile
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

import { RrdPageConfig } from '~/app/core/components/intuition/models/rrd-page-config.type';

@Component({
  template: '<omv-intuition-rrd-page [config]="this.config"></omv-intuition-rrd-page>'
})
export class PerformanceStatisticDfRrdPageComponent {
  public config: RrdPageConfig = {
    graphs: [
      {
        name: 'df-{{ "root" if mountpoint == "/" else mountpoint | substr(1) | replace("/", "-") }}'
      }
    ],
    label:
      '{{ devicefile }}{{ " [" + label + "]" if label }}{{ " [" + "System" | translate + "]" if mountpoint == "/" }}',
    store: {
      proxy: {
        service: 'FileSystemMgmt',
        get: {
          method: 'enumerateMountedFilesystems',
          params: {
            includeroot: true
          }
        }
      },
      sorters: [
        {
          prop: 'devicefile',
          dir: 'asc'
        }
      ]
    }
  };
}
