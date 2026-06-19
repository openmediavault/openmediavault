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
import {
  HttpEvent,
  HttpHandler,
  HttpInterceptor,
  HttpRequest,
  HttpResponse
} from '@angular/common/http';
import { Injectable } from '@angular/core';
import * as _ from 'lodash';
import { Observable } from 'rxjs';
import { tap } from 'rxjs/operators';

import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { RpcResponse } from '~/app/shared/services/rpc.service';

interface SessionRpcResponse {
  status?: string;
  username?: any;
  permissions?: any;
}

/**
 * HTTP interceptor that automatically manages session state for Session RPC calls.
 *
 * Handles:
 * - Session.authenticate: Sets session when status is 'authenticated'.
 * - Session.verify: Sets session after successful MFA verification.
 * - Session.logout: Revokes session.
 */
@Injectable({
  providedIn: 'root'
})
export class HttpSessionInterceptorService implements HttpInterceptor {
  constructor(private authSessionService: AuthSessionService) {}

  intercept(request: HttpRequest<any>, next: HttpHandler): Observable<HttpEvent<any>> {
    return next.handle(request).pipe(
      tap((event: HttpEvent<any>) => {
        if (event instanceof HttpResponse) {
          this.handleResponse(request, event);
        }
      })
    );
  }

  /**
   * Handle HTTP responses and update session if it's a successful 'Session' RPC.
   */
  private handleResponse(request: HttpRequest<any>, response: HttpResponse<any>): void {
    if (!this.isRpcRequest(request)) {
      return;
    }

    const body = request.body;
    if (!_.isPlainObject(body)) {
      return;
    }

    if (!_.isString(body.service) || !_.isString(body.method)) {
      return;
    }

    if (body.service !== 'Session') {
      return;
    }

    if (!['authenticate', 'verify', 'logout'].includes(body.method)) {
      return;
    }

    switch (body.method) {
      case 'logout':
        this.handleLogoutResponse();
        break;
      case 'authenticate':
      case 'verify':
        this.handleSessionResponse(body.method, response);
        break;
    }
  }

  /**
   * Check if the request is an RPC call to rpc.php endpoint.
   * Ensures the URL ends with 'rpc.php' or contains 'rpc.php?' to avoid false positives.
   */
  private isRpcRequest(request: HttpRequest<any>): boolean {
    return /rpc\.php(\?|$)/.test(request.url);
  }

  /**
   * Handle Session.authenticate and Session.verify responses.
   */
  private handleSessionResponse(
    method: 'authenticate' | 'verify',
    response: HttpResponse<any>
  ): void {
    const responseBody = response.body as RpcResponse;
    if (!_.isPlainObject(responseBody) || !_.isPlainObject(responseBody.response)) {
      return;
    }

    const rpcResponse = responseBody.response as SessionRpcResponse;
    if (method === 'authenticate') {
      this.handleAuthenticateResponse(rpcResponse);
    } else {
      this.handleVerifyResponse(rpcResponse);
    }
  }

  /**
   * Handle 'Session.authenticate' response.
   * Only set session if status is explicitly 'authenticated' (not 'challengeRequired').
   * Validates that username is a string and permissions is a valid object.
   */
  private handleAuthenticateResponse(response: SessionRpcResponse): void {
    if (response.status !== 'authenticated') {
      return;
    }

    if (_.isString(response.username) && _.isPlainObject(response.permissions)) {
      this.authSessionService.set(response.username, response.permissions);
    }
  }

  /**
   * Handle 'Session.verify' response.
   * Always set session if response contains valid username and permissions.
   * This indicates successful completion of the authentication flow (including MFA).
   */
  private handleVerifyResponse(response: SessionRpcResponse): void {
    if (_.isString(response.username) && _.isPlainObject(response.permissions)) {
      this.authSessionService.set(response.username, response.permissions);
    }
  }

  /**
   * Handle 'Session.logout' response.
   * Revokes the current session (clears session storage).
   */
  private handleLogoutResponse(): void {
    this.authSessionService.revoke();
  }
}
