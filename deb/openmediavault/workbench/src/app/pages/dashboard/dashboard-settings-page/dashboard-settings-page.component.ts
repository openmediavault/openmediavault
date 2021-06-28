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
import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { SelectionListPageConfig } from '~/app/core/components/limn-ui/models/selection-list-page-config.type';
import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';

type SelectionListItem = Pick<DashboardWidgetConfig, 'id' | 'title'> & {
  enabled: boolean;
};

@Component({
  template: '<omv-limn-selection-list-page [config]="this.config"></omv-limn-selection-list-page>'
})
export class DashboardSettingsPageComponent implements OnInit {
  public config: SelectionListPageConfig = {
    title: gettext('Enabled widgets'),
    multiple: true,
    textProp: 'title',
    valueProp: 'id',
    selectedProp: 'enabled',
    updateStoreOnSelectionChange: true,
    store: {
      data: [],
      sorters: [
        {
          dir: 'asc',
          prop: 'title'
        }
      ]
    },
    buttons: [
      {
        template: 'submit',
        execute: {
          type: 'click',
          click: this.onSubmit.bind(this)
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/dashboard'
        }
      }
    ]
  };

  constructor(
    private dashboardWidgetConfigService: DashboardWidgetConfigService,
    private router: Router
  ) {}

  ngOnInit(): void {
    this.dashboardWidgetConfigService.configs$.subscribe(
      (widgets: Array<DashboardWidgetConfig>) => {
        const data: Array<SelectionListItem> = [];
        const enabledWidgets: Array<string> = this.dashboardWidgetConfigService.getEnabled();
        _.forEach(widgets, (widget: DashboardWidgetConfig) => {
          data.push({
            id: widget.id,
            title: widget.title,
            enabled: enabledWidgets.includes(widget.id)
          });
        });
        this.config.store.data = [...data];
      }
    );
  }

  onSubmit(buttonConfig, store, value: Array<string>) {
    this.dashboardWidgetConfigService.setEnabled(value);
    this.router.navigate(['/dashboard']);
  }
}
