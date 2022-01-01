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
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import { Observable } from 'rxjs';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { DataStoreResponse, DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-dashboard-widget-services-status',
  templateUrl: './widget-services-status.component.html',
  styleUrls: ['./widget-services-status.component.scss']
})
export class WidgetServicesStatusComponent extends AbstractDashboardWidgetComponent<DataStoreResponse> {
  tooltipText = {
    true: gettext('Running'),
    false: gettext('Not running')
  };

  constructor(private dataStoreService: DataStoreService) {
    super();
  }

  protected loadData(): Observable<DataStoreResponse> {
    return this.dataStoreService.load({
      proxy: {
        service: 'Services',
        get: {
          method: 'getStatus'
        }
      },
      filters: [
        {
          operator: 'truthy',
          arg0: { prop: 'enabled' }
        }
      ],
      sorters: [
        {
          prop: 'title',
          dir: 'asc'
        }
      ]
    });
  }
}
