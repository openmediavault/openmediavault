/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import * as _ from 'lodash';
import { Observable } from 'rxjs';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { DataStoreResponse } from '~/app/shared/services/data-store.service';
import { DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-dashboard-widget-datatable',
  templateUrl: './widget-datatable.component.html',
  styleUrls: ['./widget-datatable.component.scss']
})
export class WidgetDatatableComponent extends AbstractDashboardWidgetComponent<DataStoreResponse> {
  constructor(private dataStoreService: DataStoreService) {
    super();
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      reloadPeriod: 10000,
      datatable: {
        columnMode: 'flex',
        hasHeader: true,
        hasFooter: false,
        columns: [],
        store: {
          data: []
        }
      }
    });
  }

  protected loadData(): Observable<DataStoreResponse> {
    return this.dataStoreService.load(this.config.datatable.store);
  }
}
