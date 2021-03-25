import { Directive, EventEmitter, Input, OnDestroy, OnInit, Output } from '@angular/core';
import * as _ from 'lodash';
import { EMPTY, Observable, Subscription } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';

@Directive()
// eslint-disable-next-line @angular-eslint/directive-class-suffix
export abstract class AbstractDashboardWidgetComponent<T> implements OnInit, OnDestroy {
  @Input()
  config: DashboardWidgetConfig;

  @Output()
  readonly loadDataEvent = new EventEmitter<T>();

  data?: T;
  loading = false;
  firstLoad = true;
  error: any | boolean = false;

  protected subscriptions: Subscription = new Subscription();

  ngOnInit(): void {
    this.sanitizeConfig();
    this.reloadData();
  }

  ngOnDestroy(): void {
    this.subscriptions.unsubscribe();
  }

  protected sanitizeConfig(): void {}

  protected isAutoReloadable(): boolean {
    return _.isNumber(this.config?.reloadPeriod) && this.config.reloadPeriod > 0;
  }

  protected reloadData(): void {
    this.loading = true;
    this.subscriptions.add(
      this.loadData()
        .pipe(
          catchError((error) => {
            // Do not show an error notification.
            if (_.isFunction(error.preventDefault)) {
              error.preventDefault();
            }
            this.error = error;
            return EMPTY;
          }),
          finalize(() => {
            this.loading = false;
            // Reload the data after N seconds.
            if (this.isAutoReloadable()) {
              setTimeout(() => {
                this.reloadData();
              }, this.config.reloadPeriod);
            }
          })
        )
        .subscribe((data) => {
          this.error = false;
          this.firstLoad = false;
          this.data = data;
          this.loadDataEvent.emit(data);
        })
    );
  }

  protected abstract loadData(): Observable<T>;
}
