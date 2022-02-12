import { Component } from '@angular/core';
import * as _ from 'lodash';
import { Observable } from 'rxjs';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { DataStoreResponse, DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-dashboard-widget-grid',
  templateUrl: './widget-grid.component.html',
  styleUrls: ['./widget-grid.component.scss']
})
export class WidgetGridComponent extends AbstractDashboardWidgetComponent<DataStoreResponse> {
  constructor(private dataStoreService: DataStoreService) {
    super();
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      reloadPeriod: 10000,
      grid: {
        store: {
          data: []
        }
      }
    });
  }

  protected loadData(): Observable<DataStoreResponse> {
    return this.dataStoreService.load(this.config.grid.store);
  }
}
