import { Injectable } from '@angular/core';
import * as _ from 'lodash';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';

@Injectable({
  providedIn: 'root'
})
export class UserStorageService {
  constructor(private authSessionService: AuthSessionService) {}

  get(key: string, defaultValue?: any): string | null {
    const username = this.authSessionService.getUsername();
    const value = localStorage.getItem(`${username}@${key}`);
    return _.defaultTo(value, defaultValue);
  }

  set(key: string, value: string) {
    const username = this.authSessionService.getUsername();
    localStorage.setItem(`${username}@${key}`, value);
  }

  clear() {
    const username = this.authSessionService.getUsername();
    const numKeys = localStorage.length;
    for (let i = 0; i < numKeys; i++) {
      const key = localStorage.key(i);
      if (_.startsWith(key, `${username}@`)) {
        localStorage.removeItem(key);
      }
    }
  }
}
