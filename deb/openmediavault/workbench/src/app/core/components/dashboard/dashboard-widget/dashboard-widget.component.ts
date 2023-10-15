import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import * as _ from 'lodash';
import { EMPTY, Observable, Subscription, timer } from 'rxjs';
import { catchError, finalize, map, take } from 'rxjs/operators';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { Unsubscribe } from '~/app/decorators';
import { formatDeep } from '~/app/functions.helper';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-dashboard-widget',
  templateUrl: './dashboard-widget.component.html',
  styleUrls: ['./dashboard-widget.component.scss']
})
export class DashboardWidgetComponent implements OnInit {
  @Input()
  config: DashboardWidgetConfig;

  @Input()
  loadData?: () => Observable<any>;

  @Output()
  readonly dataChangedEvent = new EventEmitter<any>();

  @Unsubscribe()
  private subscriptions: Subscription = new Subscription();

  public loading = false;
  public firstLoad = true;
  public error: any | boolean = false;

  constructor(private rpcService: RpcService) {}

  ngOnInit(): void {
    this.sanitizeConfig();
    this.reloadData();
  }

  protected sanitizeConfig() {
    _.defaultsDeep(this.config, {
      hideTitle: false
    });
  }

  protected isAutoReloadable(): boolean {
    return _.isNumber(this.config?.reloadPeriod) && this.config.reloadPeriod > 0;
  }

  protected getLoadData(): Observable<any> {
    if (_.isFunction(this.loadData)) {
      return this.loadData();
    }
    if (!_.isPlainObject(this.config)) {
      return EMPTY;
    }
    const type = this.config.type;
    const request: Record<string, any> = _.get(this.config, `${type}.request`);
    if (!_.isPlainObject(request)) {
      return EMPTY;
    }
    return this.rpcService[request.task ? 'requestTask' : 'request'](
      request.service,
      request.method,
      request.params
    ).pipe(
      map((res) => {
        if (_.isPlainObject(request.transform)) {
          const tmp = formatDeep(request.transform, res);
          _.merge(res, tmp);
        }
        return res;
      })
    );
  }

  protected reloadData(): void {
    this.loading = true;
    // Store the subscription to clean it up correctly when the
    // component is destroyed before the subscription completes.
    // `take(1)` will unsubscribe the subscription automatically
    // otherwise.
    this.subscriptions.add(
      this.getLoadData()
        .pipe(
          catchError((error) => {
            // Do not show an error notification.
            error.preventDefault?.();
            this.error = error;
            return EMPTY;
          }),
          finalize(() => {
            this.loading = false;
            this.firstLoad = false;
            // Reload the data after N seconds?
            if (this.isAutoReloadable()) {
              // Store the timer subscription to unsubscribe
              // correctly when the widget component is destroyed
              // before the subscription completes.
              this.subscriptions.add(
                timer(this.config.reloadPeriod)
                  .pipe(take(1))
                  .subscribe(() => {
                    this.reloadData();
                  })
              );
            }
          }),
          take(1)
        )
        .subscribe((data: any) => {
          this.error = false;
          this.dataChangedEvent.emit(data);
        })
    );
  }
}
