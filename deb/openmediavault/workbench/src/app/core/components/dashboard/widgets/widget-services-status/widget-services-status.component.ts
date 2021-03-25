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
