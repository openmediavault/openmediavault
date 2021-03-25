import { Component } from '@angular/core';
import { Observable } from 'rxjs';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { DataStoreResponse, DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-dashboard-widget-filesystems-status',
  templateUrl: './widget-filesystems-status.component.html',
  styleUrls: ['./widget-filesystems-status.component.scss']
})
export class WidgetFilesystemsStatusComponent extends AbstractDashboardWidgetComponent<DataStoreResponse> {
  constructor(private dataStoreService: DataStoreService) {
    super();
  }

  protected loadData(): Observable<DataStoreResponse> {
    return this.dataStoreService.load({
      proxy: {
        service: 'FileSystemMgmt',
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
          prop: 'canonicaldevicefile',
          dir: 'asc'
        }
      ]
    });
  }
}
