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

import { PageContext } from '~/app/core/models/page-context.type';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { RouteContextService } from '~/app/shared/services/route-context.service';

@Injectable()
export class PageContextService {
  private props: Record<string, any> = {};

  constructor(
    private authSessionService: AuthSessionService,
    private routeContextService: RouteContextService
  ) {}

  public set(props: Record<string, any>): PageContext {
    this.props = { ...this.props, ...props };
    return this.get();
  }

  public get(): PageContext {
    return {
      _session: {
        username: this.authSessionService.getUsername(),
        permissions: this.authSessionService.getPermissions()
      },
      ...this.routeContextService.get(),
      ...this.props
    };
  }
}
