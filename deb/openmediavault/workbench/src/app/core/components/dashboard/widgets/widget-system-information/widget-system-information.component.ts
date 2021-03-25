import { Component } from '@angular/core';
import dayjs from 'dayjs';
import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';

@Component({
  selector: 'omv-dashboard-widget-system-information',
  templateUrl: './widget-system-information.component.html',
  styleUrls: ['./widget-system-information.component.scss']
})
export class WidgetSystemInformationComponent extends AbstractDashboardWidgetComponent<SystemInformation> {
  constructor(private systemInformationService: SystemInformationService) {
    super();
  }

  protected loadData(): Observable<SystemInformation> {
    return this.systemInformationService.systemInfo$.pipe(
      map((data) => {
        data.uptime = dayjs().unix() - data.uptime;
        return data;
      })
    );
  }
}
