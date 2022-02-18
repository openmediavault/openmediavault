import { Component } from '@angular/core';
import * as _ from 'lodash';
import { EMPTY, Observable } from 'rxjs';

import { AbstractDashboardWidgetComponent } from '~/app/core/components/dashboard/widgets/abstract-dashboard-widget-component';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-dashboard-widget-text',
  templateUrl: './widget-text.component.html',
  styleUrls: ['./widget-text.component.scss']
})
export class WidgetTextComponent extends AbstractDashboardWidgetComponent<string> {
  constructor(private rpcService: RpcService) {
    super();
  }

  protected loadData(): Observable<string> {
    const request = this.config.text?.request;
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
