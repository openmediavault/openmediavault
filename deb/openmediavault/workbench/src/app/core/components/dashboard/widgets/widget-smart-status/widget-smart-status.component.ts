/* eslint-disable @typescript-eslint/naming-convention */
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
import { Observable } from 'rxjs';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { DataStoreResponse, DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-dashboard-widget-smart-status',
  templateUrl: './widget-smart-status.component.html',
  styleUrls: ['./widget-smart-status.component.scss']
})
export class WidgetSmartStatusComponent extends AbstractDashboardWidgetComponent<DataStoreResponse> {
  tooltipText = {
    GOOD: gettext('Good'),
    BAD_STATUS: gettext('Unknown'),
    BAD_ATTRIBUTE_NOW: gettext('Device is being used outside design parameters.'),
    BAD_ATTRIBUTE_IN_THE_PAST: gettext('Device was used outside of design parameters in the past.'),
    BAD_SECTOR: gettext('Device has a few bad sectors.'),
    BAD_SECTOR_MANY: gettext('Device has many bad sectors.')
  };

  constructor(private dataStoreService: DataStoreService) {
    super();
  }

  protected loadData(): Observable<DataStoreResponse> {
    return this.dataStoreService.load({
      proxy: {
        service: 'Smart',
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
          prop: 'devicefile',
          dir: 'asc'
        }
      ]
    });
  }
}
