/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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

import { Permissions, Roles } from '~/app/shared/models/permissions.model';

@Injectable({
  providedIn: 'root'
})
export class AuthSessionService {
  constructor() {}

  /**
   * Stores the username and permissions in session storage.
   * @param username The username to store.
   * @param permissions The permissions to store.
   */
  set(username: string, permissions: Permissions): void {
    sessionStorage.setItem('username', username);
    sessionStorage.setItem('permissions', Permissions.toJSON(permissions));
  }

  /**
   * Returns the stored username, if present.
   * @returns The stored username, or null if none exists.
   */
  getUsername(): string | null {
    return sessionStorage.getItem('username');
  }

  /**
   * Returns the stored permissions or an empty permissions object.
   * @returns The stored permissions object.
   */
  getPermissions(): Permissions {
    const item = sessionStorage.getItem('permissions') || '{}';
    return Permissions.fromJSON(item);
  }

  /**
   * Removes the stored auth session data.
   */
  revoke(): void {
    sessionStorage.removeItem('username');
    sessionStorage.removeItem('permissions');
  }

  /**
   * Checks whether the current session includes the admin role.
   * @returns True if the admin role is present, otherwise false.
   */
  hasAdminRole(): boolean {
    const permissions = this.getPermissions();
    return (permissions.role ?? []).includes(Roles.admin);
  }
}
