import { Injectable } from '@angular/core';

import { Permissions } from '~/app/shared/models/permissions.model';

@Injectable({
  providedIn: 'root'
})
export class AuthSessionService {
  constructor() {}

  set(username: string, permissions: Permissions): void {
    localStorage.setItem('username', username);
    localStorage.setItem('permissions', Permissions.toJSON(permissions));
  }

  getUsername(): string {
    return localStorage.getItem('username');
  }

  getPermissions(): Permissions {
    const item = localStorage.getItem('permissions') || '{}';
    return Permissions.fromJSON(item);
  }

  revoke(): void {
    localStorage.removeItem('username');
    localStorage.removeItem('permissions');
  }
}
