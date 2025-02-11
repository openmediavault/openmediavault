/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
import * as _ from 'lodash';
import { EMPTY, Observable, ReplaySubject, Subscription, timer } from 'rxjs';
import { catchError, exhaustMap, filter } from 'rxjs/operators';

import { AuthService } from '~/app/shared/services/auth.service';
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
  availablePkgUpdates?: number;
  displayWelcomeMessage?: boolean;
};

@Injectable({
  providedIn: 'root'
})
export class SystemInformationService implements OnDestroy {
  public readonly systemInfo$: Observable<SystemInformation>;

  private subscription: Subscription;
  private systemInfoSource = new ReplaySubject<SystemInformation>(1);

  constructor(
    private authService: AuthService,
    private rpcService: RpcService
  ) {
    this.systemInfo$ = this.systemInfoSource.asObservable();
    // Poll the system system-information every 5 seconds. Continue, even
    // if there is a connection problem AND do not display an error
    // notification.
    this.subscription = timer(0, 5000)
      .pipe(
        // Do not request system information if user is not logged in.
        filter(() => this.authService.isLoggedIn()),
        // Request the system information via HTTP. Execute the RPC only
        // after the previous one has been completed.
        exhaustMap(() =>
          this.rpcService
            .request('System', 'getInformation', null, { updatelastaccess: false })
            .pipe(
              catchError((error) => {
                // Do not show an error notification.
                error.preventDefault?.();
                return EMPTY;
              })
            )
        )
      )
      .subscribe({
        next: (res: SystemInformation) => {
          if (!_.isPlainObject(res)) {
            return;
          }
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
        }
      });
  }

  ngOnDestroy(): void {
    this.subscription.unsubscribe();
  }
}
