import { Component } from '@angular/core';
import * as _ from 'lodash';
import { Observable, of } from 'rxjs';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { unixTimeStamp } from '~/app/functions.helper';

@Component({
  selector: 'omv-dashboard-widget-rrd',
  templateUrl: './widget-rrd.component.html',
  styleUrls: ['./widget-rrd.component.scss']
})
export class WidgetRrdComponent extends AbstractDashboardWidgetComponent<number> {
  public time: number;

  constructor() {
    super();
    this.subscriptions.add(
      this.loadDataEvent.subscribe((time: number) => {
        this.time = time;
      })
    );
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      reloadPeriod: 60000,
      rrd: {
        name: 'undefined.png'
      }
    });
  }

  protected loadData(): Observable<number> {
    // Angular CD will detect this and redraws the widget using the
    // latest graph image.
    return of(unixTimeStamp());
  }
}
