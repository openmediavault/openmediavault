import { Component } from '@angular/core';
import * as _ from 'lodash';
import { EMPTY, Observable } from 'rxjs';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-dashboard-widget-value',
  templateUrl: './widget-value.component.html',
  styleUrls: ['./widget-value.component.scss']
})
export class WidgetValueComponent extends AbstractDashboardWidgetComponent<any> {
  constructor(private rpcService: RpcService) {
    super();
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      reloadPeriod: 10000
    });
  }

  protected loadData(): Observable<any> {
    const request = this.config.value?.request;
    if (!_.isPlainObject(request)) {
      return EMPTY;
    }
    return this.rpcService[request.task ? 'requestTask' : 'request'](
      request.service,
      request.method,
      request.params
    );
  }
}
