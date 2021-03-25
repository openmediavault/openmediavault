import { Component, OnInit } from '@angular/core';
import * as _ from 'lodash';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { DashboardWidgetConfigService } from '~/app/core/services/dashboard-widget-config.service';

@Component({
  selector: 'omv-dashboard-page',
  templateUrl: './dashboard-page.component.html',
  styleUrls: ['./dashboard-page.component.scss']
})
export class DashboardPageComponent implements OnInit {
  public widgets: Array<DashboardWidgetConfig> = [];

  constructor(private dashboardWidgetConfigService: DashboardWidgetConfigService) {}

  ngOnInit(): void {
    this.dashboardWidgetConfigService.configs$.subscribe(
      (widgets: Array<DashboardWidgetConfig>) => {
        const enabledWidgets: Array<string> = this.dashboardWidgetConfigService.getEnabled();
        this.widgets = [];
        _.forEach(enabledWidgets, (id) => {
          const widget = _.find(widgets, ['id', id]);
          if (!_.isUndefined(widget)) {
            this.widgets.push(widget);
          }
        });
      }
    );
  }
}
