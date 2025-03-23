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
import { Component, Input, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { Observable } from 'rxjs';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { formatDeep, isFormatable } from '~/app/functions.helper';
import { DataStoreResponse, DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-dashboard-widget-grid',
  templateUrl: './dashboard-widget-grid.component.html',
  styleUrls: ['./dashboard-widget-grid.component.scss']
})
export class DashboardWidgetGridComponent implements OnInit {
  @Input()
  config: DashboardWidgetConfig;

  public data: DataStoreResponse;

  constructor(
    private dataStoreService: DataStoreService,
    private router: Router
  ) {}

  ngOnInit(): void {
    this.sanitizeConfig();
  }

  public loadData(): Observable<DataStoreResponse> {
    return this.dataStoreService.load(this.config.grid.store);
  }

  public dataChanged(data: DataStoreResponse): void {
    this.data = data;
  }

  public onClick(data: Record<any, any>): void {
    if (isFormatable(this.config.grid?.item.url)) {
      const url = formatDeep(this.config.grid.item.url, data);
      this.router.navigate([url]);
    }
  }

  protected sanitizeConfig() {
    _.defaultsDeep(this.config, {
      reloadPeriod: 10000,
      grid: {
        item: {
          minWidth: '100px'
        },
        hasEmptyMessage: true,
        emptyMessage: gettext('No data to display.'),
        store: {
          data: []
        }
      }
    });
  }
}
