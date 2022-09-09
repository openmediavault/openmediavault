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
import { Component, Input } from '@angular/core';
import dayjs from 'dayjs';
import * as _ from 'lodash';
import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';

@Component({
  selector: 'omv-dashboard-widget-system-information',
  templateUrl: './dashboard-widget-system-information.component.html',
  styleUrls: ['./dashboard-widget-system-information.component.scss']
})
export class DashboardWidgetSystemInformationComponent {
  @Input()
  config: DashboardWidgetConfig;

  public data: SystemInformation;

  constructor(private systemInformationService: SystemInformationService) {}

  public loadData(): Observable<SystemInformation> {
    return this.systemInformationService.systemInfo$.pipe(
      map((data) => {
        const result = _.cloneDeep(data);
        result.uptime = dayjs().unix() - result.uptime;
        return result;
      })
    );
  }

  public dataChanged(data: any): void {
    this.data = data;
  }
}
