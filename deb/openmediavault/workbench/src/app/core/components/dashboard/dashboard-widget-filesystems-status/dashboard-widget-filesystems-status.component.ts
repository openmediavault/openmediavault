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
import { Component, Input } from '@angular/core';
import { Observable } from 'rxjs';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { DataStoreResponse, DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-dashboard-widget-filesystems-status',
  templateUrl: './dashboard-widget-filesystems-status.component.html',
  styleUrls: ['./dashboard-widget-filesystems-status.component.scss']
})
export class DashboardWidgetFilesystemsStatusComponent {
  @Input()
  config: DashboardWidgetConfig;

  public data: DataStoreResponse;

  constructor(private dataStoreService: DataStoreService) {}

  public loadData(): Observable<DataStoreResponse> {
    return this.dataStoreService.load({
      proxy: {
        service: 'FileSystemMgmt',
        get: {
          method: 'getListBg',
          params: {
            start: 0,
            limit: -1
          },
          task: true
        }
      },
      sorters: [
        {
          prop: 'canonicaldevicefile',
          dir: 'asc'
        }
      ]
    });
  }

  public dataChanged(data: DataStoreResponse): void {
    this.data = data;
  }
}
