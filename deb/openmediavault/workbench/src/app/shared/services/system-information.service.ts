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
import { Injectable, OnDestroy } from '@angular/core';
import { Router } from '@angular/router';
import * as _ from 'lodash';
import { EMPTY, Observable, of, ReplaySubject, Subscription } from 'rxjs';
import { catchError, delay, filter, repeat, switchMap, tap } from 'rxjs/operators';

import { RpcService } from '~/app/shared/services/rpc.service';

export type SystemInformation = {
  ts: number;
  time: string;
  hostname: string;
  version?: string;
  cpuModelName?: string;
  cpuUsage?: number;
  memTotal?: number;
  memUsed?: number;
  memAvailable?: number;
  memUtilization?: number;
  kernel?: string;
  uptime?: number;
  loadAverage?: string;
  configDirty?: boolean;
  rebootRequired?: boolean;
  pkgUpdatesAvailable?: boolean;
};

@Injectable({
  providedIn: 'root'
})
export class SystemInformationService implements OnDestroy {
  public readonly systemInfo$: Observable<SystemInformation>;

  private subscription: Subscription;
  private systemInfoSource = new ReplaySubject<SystemInformation>(1);

  constructor(private router: Router, private rpcService: RpcService) {
    this.systemInfo$ = this.systemInfoSource.asObservable();
    // Poll the system system-information every 5 seconds. Continue, even
    // if there is a connection problem AND do not display an error
    // notification.
    this.subscription = of(true)
      .pipe(
        // Do not request system information if we are at the log-in page.
        filter(() => this.router.url !== '/login'),
        // Request the system information via HTTP.
        switchMap(() =>
          this.rpcService
            .request('System', 'getInformation', null, { updatelastaccess: false })
            .pipe(
              catchError((error) => {
                // Do not show an error notification.
                if (_.isFunction(error.preventDefault)) {
                  error.preventDefault();
                }
                return EMPTY;
              })
            )
        ),
        // Notify subscribers.
        tap((res: SystemInformation) => {
          // We need to convert some properties to numbers because
          // they are strings due to the 32bit compatibility of the
          // PHP backend.
          if (_.isString(res.memUsed)) {
            res.memUsed = _.parseInt(res.memUsed);
          }
          if (_.isString(res.memTotal)) {
            res.memTotal = _.parseInt(res.memTotal);
          }
          if (_.isString(res.memAvailable)) {
            res.memAvailable = _.parseInt(res.memAvailable);
          }
          if (_.isString(res.memUtilization)) {
            res.memUtilization = Number.parseFloat(res.memUtilization);
          }
          this.systemInfoSource.next(res);
        }),
        // Delay 5 seconds before performing the next request.
        delay(5000),
        repeat()
      )
      .subscribe();
  }

  ngOnDestroy(): void {
    this.subscription.unsubscribe();
  }
}
