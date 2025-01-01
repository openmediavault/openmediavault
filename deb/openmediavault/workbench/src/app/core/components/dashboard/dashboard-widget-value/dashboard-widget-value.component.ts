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

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';

@Component({
  selector: 'omv-dashboard-widget-value',
  templateUrl: './dashboard-widget-value.component.html',
  styleUrls: ['./dashboard-widget-value.component.scss']
})
export class DashboardWidgetValueComponent implements OnInit {
  @Input()
  config: DashboardWidgetConfig;

  public data: any;

  constructor() {}

  ngOnInit(): void {
    this.sanitizeConfig();
  }

  public dataChanged(data: any): void {
    this.data = data;
  }

  protected sanitizeConfig() {
    _.defaultsDeep(this.config, {
      hideTitle: true,
      reloadPeriod: 10000
    });
  }
}
