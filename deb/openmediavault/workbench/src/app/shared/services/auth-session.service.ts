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
import { Injectable } from '@angular/core';

import { Permissions, Roles } from '~/app/shared/models/permissions.model';

@Injectable({
  providedIn: 'root'
})
export class AuthSessionService {
  constructor() {}

  set(username: string, permissions: Permissions): void {
    sessionStorage.setItem('username', username);
    sessionStorage.setItem('permissions', Permissions.toJSON(permissions));
  }

  getUsername(): string | null {
    return sessionStorage.getItem('username');
  }

  getPermissions(): Permissions {
    const item = sessionStorage.getItem('permissions') || '{}';
    return Permissions.fromJSON(item);
  }

  revoke(): void {
    sessionStorage.removeItem('username');
    sessionStorage.removeItem('permissions');
  }

  hasAdminRole(): boolean {
    const permissions = this.getPermissions();
    return permissions.role.includes(Roles.admin);
  }
}
