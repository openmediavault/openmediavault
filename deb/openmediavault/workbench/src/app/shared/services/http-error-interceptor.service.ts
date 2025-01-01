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
import {
  HttpErrorResponse,
  HttpEvent,
  HttpHandler,
  HttpInterceptor,
  HttpRequest
} from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Router } from '@angular/router';
import * as _ from 'lodash';
import { Observable, throwError } from 'rxjs';
import { catchError } from 'rxjs/operators';

import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { NotificationService } from '~/app/shared/services/notification.service';

@Injectable({
  providedIn: 'root'
})
export class HttpErrorInterceptorService implements HttpInterceptor {
  constructor(
    private router: Router,
    private authSessionService: AuthSessionService,
    private notificationService: NotificationService
  ) {}

  intercept(request: HttpRequest<any>, next: HttpHandler): Observable<HttpEvent<any>> {
    return next.handle(request).pipe(
      catchError((err) => {
        if (err instanceof HttpErrorResponse) {
          let title = `${err.status} - ${err.statusText}`;
          let message: string;
          let traceback: string;

          // Convert backend errors.
          if (_.isPlainObject(err.error) && _.isPlainObject(err.error.error)) {
            _.merge(err.error, {
              message: err.error.error.message,
              traceback: err.error.error.trace
            });
            delete err.error.response;
            delete err.error.error;
          }

          if (_.isPlainObject(err.error)) {
            title = _.defaultTo(err.error.detail, `${err.status} - ${err.statusText}`);
            message = _.defaultTo(err.error.message, '');
            traceback = _.defaultTo(err.error.traceback, '');
          }

          switch (err.status) {
            case 0:
            case 503:
              this.router.navigate(['/503']);
              break;
            case 401:
              this.authSessionService.revoke();
              this.router.navigate(['/login']);
              break;
            default:
              // Show the notification.
              // The notification can be canceled within 5 milliseconds.
              const notificationId: number = this.notificationService.show(
                NotificationType.error,
                title,
                message,
                traceback
              );
              // Add a method to allow the subscriber to cancel the
              // notification, so it will not be shown.
              _.set(err, 'preventDefault', () => {
                this.notificationService.cancel(notificationId);
              });
              break;
          }
        }
        return throwError(err);
      })
    );
  }
}
