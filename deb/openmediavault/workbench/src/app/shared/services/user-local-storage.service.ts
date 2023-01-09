/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2023 Volker Theile
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
import { Injectable } from '@angular/core';
import * as _ from 'lodash';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';

@Injectable({
  providedIn: 'root'
})
export class UserLocalStorageService {
  constructor(private authSessionService: AuthSessionService) {}

  get(key: string, defaultValue?: any): string | null {
    const username = this.authSessionService.getUsername();
    const value = localStorage.getItem(`${username}@${key}`);
    return _.defaultTo(value, defaultValue);
  }

  set(key: string, value: string): void {
    const username = this.authSessionService.getUsername();
    localStorage.setItem(`${username}@${key}`, value);
  }

  remove(key: string): void {
    const username = this.authSessionService.getUsername();
    localStorage.removeItem(`${username}@${key}`);
  }

  clear(): void {
    const username = this.authSessionService.getUsername();
    const numKeys = localStorage.length;
    const keys = [];
    for (let i = 0; i < numKeys; i++) {
      const key = localStorage.key(i);
      if (_.startsWith(key, `${username}@`)) {
        keys.push(key);
      }
    }
    _.forEach(keys, (key) => localStorage.removeItem(key));
  }
}
