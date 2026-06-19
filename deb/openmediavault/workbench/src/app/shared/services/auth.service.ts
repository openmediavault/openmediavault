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
import * as _ from 'lodash';
import { Observable } from 'rxjs';
import { finalize } from 'rxjs/operators';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { PrefersColorSchemeService } from '~/app/shared/services/prefers-color-scheme.service';
import { RpcService } from '~/app/shared/services/rpc.service';

export type SessionData = {
  authenticated: boolean;
  permissions: { [key: string]: any };
  sessionid: string;
  username: string;
};

export type ChallengeInfo = {
  [key: string]: any;
  kind: string;
  redirecturl?: string;
};

export type AuthenticateResponse = {
  status: 'authenticated' | 'challengeRequired';
  username: string;
  sessionid?: string;
  permissions?: { [key: string]: any };
  challenge?: ChallengeInfo;
};

@Injectable({
  providedIn: 'root'
})
export class AuthService {
  constructor(
    private authSessionService: AuthSessionService,
    private rpcService: RpcService,
    private prefersColorSchemeService: PrefersColorSchemeService
  ) {}

  /**
   * Step 1 of 2-step login: Authenticate with username/password.
   * May return a challenge (e.g., MFA) that needs verification.
   *
   * Note: Session handling is now done automatically by HttpSessionInterceptorService,
   * which intercepts all successful Session.authenticate HTTP responses.
   */
  authenticate(username: string, password: string): Observable<AuthenticateResponse> {
    return this.rpcService.request('Session', 'authenticate', {
      username,
      password
    });
  }

  logout(): Observable<void> {
    // Always logout in case of success AND failure.
    //
    // Note: Session revocation is now done automatically by HttpSessionInterceptorService,
    // which intercepts all successful Session.logout HTTP responses.
    return this.rpcService.request('Session', 'logout').pipe(
      finalize(() => {
        // Reload the page. The Angular router and auth-guard service will
        // redirect automatically to the login page.
        // Reset the color scheme to the default.
        this.prefersColorSchemeService.reset();
        document.location.replace('');
      })
    );
  }

  isLoggedIn(): boolean {
    return !_.isNull(this.authSessionService.getUsername());
  }
}
