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
import * as _ from 'lodash';
import { ToastrService } from 'ngx-toastr';
import { BehaviorSubject, Observable } from 'rxjs';

import { translate } from '~/app/i18n.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { Notification } from '~/app/shared/models/notification.model';

@Injectable({
  providedIn: 'root'
})
export class NotificationService {
  public readonly notifications$: Observable<Notification[]>;

  private notificationsSource = new BehaviorSubject<Notification[]>([]);

  constructor(private toastrService: ToastrService) {
    this.notifications$ = this.notificationsSource.asObservable();
  }

  show(notification: Notification): number;
  show(type: NotificationType, title: string, message?: string, traceback?: string): number;
  show(
    arg: Notification | NotificationType,
    title?: string,
    message?: string,
    traceback?: string
  ): number {
    return window.setTimeout(() => {
      let notification: Notification;
      if (arg instanceof Notification) {
        notification = arg as Notification;
      } else {
        notification = new Notification(arg as NotificationType, title, message, traceback);
      }
      // Store the notification.
      this.add(notification);
      // Show the notification as toasty.
      const fn: (message: string, title: string) => any = _.bind(
        this.toastrService[notification.type],
        this.toastrService
      );
      fn(
        _.truncate(translate(notification.message), { length: 1500, omission: '...' }),
        translate(notification.title)
      );
    }, 5);
  }

  add(notification: Notification): void {
    const notifications = this.getAll();
    notifications.push(notification);
    this.notificationsSource.next(notifications);
  }

  cancel(id: number): void {
    window.clearTimeout(id);
  }

  remove(notification: Notification): void {
    const notifications = this.getAll();
    _.remove(notifications, { id: notification.id });
    this.notificationsSource.next(notifications);
  }

  removeAll(): void {
    this.notificationsSource.next([]);
  }

  getAll(): Notification[] {
    return this.notificationsSource.getValue();
  }
}
