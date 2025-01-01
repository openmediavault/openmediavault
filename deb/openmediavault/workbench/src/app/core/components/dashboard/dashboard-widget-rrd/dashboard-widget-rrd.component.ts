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
import * as _ from 'lodash';
import { Observable, of } from 'rxjs';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { unixTimeStamp } from '~/app/functions.helper';

@Component({
  selector: 'omv-dashboard-widget-rrd',
  templateUrl: './dashboard-widget-rrd.component.html',
  styleUrls: ['./dashboard-widget-rrd.component.scss']
})
export class DashboardWidgetRrdComponent implements OnInit {
  @Input()
  config: DashboardWidgetConfig;

  public time: number;

  constructor() {}

  ngOnInit(): void {
    this.sanitizeConfig();
  }

  public loadData(): Observable<number> {
    // Angular CD will detect this and redraws the widget using the
    // latest graph image.
    return of(unixTimeStamp());
  }

  public dataChanged(time: number): void {
    this.time = time;
  }

  protected sanitizeConfig() {
    _.defaultsDeep(this.config, {
      reloadPeriod: 60000,
      rrd: {
        name: 'undefined.png'
      }
    });
  }
}
