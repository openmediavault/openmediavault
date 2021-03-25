import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { finalize, tap } from 'rxjs/operators';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { RpcService } from '~/app/shared/services/rpc.service';

export type SessionData = {
  authenticated: boolean;
  permissions: { [key: string]: any };
  username: string;
};

@Injectable({
  providedIn: 'root'
})
export class AuthService {
  constructor(private authSessionService: AuthSessionService, private rpcService: RpcService) {}

  login(username: string, password: string): Observable<SessionData> {
    return this.rpcService
      .request('Session', 'login', {
        username,
        password
      })
      .pipe(
        tap((res: SessionData) => {
          this.authSessionService.set(res.username, res.permissions);
        })
      );
  }

  logout(): Observable<void> {
    // Always logout in case of success AND failure.
    return this.rpcService.request('Session', 'logout').pipe(
      finalize(() => {
        // Revoke session and reload the page. The Angular router and
        // auth-guard service will redirect automatically to the login
        // page.
        this.authSessionService.revoke();
        document.location.replace('');
      })
    );
  }

  isLoggedIn(): boolean {
    return this.authSessionService.getUsername() !== null;
  }
}
