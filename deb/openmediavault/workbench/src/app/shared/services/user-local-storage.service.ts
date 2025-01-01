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
import { Platform } from '@angular/cdk/platform';
import { Injectable } from '@angular/core';
import * as _ from 'lodash';
import { Observable, of } from 'rxjs';
import { catchError, tap } from 'rxjs/operators';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Injectable({
  providedIn: 'root'
})
export class UserLocalStorageService {
  constructor(
    private authSessionService: AuthSessionService,
    private platform: Platform,
    private rpcService: RpcService
  ) {}

  private get deviceType(): string {
    return this.platform.ANDROID || this.platform.IOS ? 'mobile' : 'desktop';
  }

  get(key: string, defaultValue?: any): string | null {
    const username = this.authSessionService.getUsername();
    const value = localStorage.getItem(`${username}@${key}`);
    return _.defaultTo(value, defaultValue);
  }

  set(key: string, value: string, localOnly: boolean = false): void {
    const username = this.authSessionService.getUsername();
    // Set local storage item ...
    // - in browser:
    localStorage.setItem(`${username}@${key}`, value);
    // - on server:
    if (!localOnly) {
      this.rpcService
        .request('WebGui', 'setLocalStorageItem', {
          devicetype: this.deviceType,
          key,
          value
        })
        .subscribe();
    }
  }

  clear(): void {
    const username = this.authSessionService.getUsername();
    // Clear local storage ...
    // - in browser:
    const keys = [];
    for (let i = 0; i < localStorage.length; i++) {
      const key = localStorage.key(i);
      if (_.startsWith(key, `${username}@`)) {
        keys.push(key);
      }
    }
    _.forEach(keys, (key) => localStorage.removeItem(key));
    // - on server:
    this.rpcService
      .request('WebGui', 'clearLocalStorageItems', {
        devicetype: this.deviceType
      })
      .subscribe();
  }

  /**
   * Load the users local storage settings and apply them to the
   * browsers local storage.
   */
  load(): Observable<Record<string, string>> {
    return this.rpcService
      .request('WebGui', 'getLocalStorageItems', {
        devicetype: this.deviceType
      })
      .pipe(
        catchError((error) => {
          error.preventDefault?.();
          return of({});
        }),
        tap((items: Record<string, string>) => {
          _.forEach(items, (value, key) => {
            this.set(key, value, true);
          });
        })
      );
  }
}
