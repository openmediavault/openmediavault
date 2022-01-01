/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import { Directive, EventEmitter, Input, OnDestroy, OnInit, Output } from '@angular/core';
import * as _ from 'lodash';
import { EMPTY, Observable, Subscription, timer } from 'rxjs';
import { catchError, finalize, take } from 'rxjs/operators';

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
    // Store the subscription to clean it up correctly when the
    // component is destroyed before the subscription completes.
    // `take(1)` will unsubscribe the subscription automatically
    // otherwise.
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
