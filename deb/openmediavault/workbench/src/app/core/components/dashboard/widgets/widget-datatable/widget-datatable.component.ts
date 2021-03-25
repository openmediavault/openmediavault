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
