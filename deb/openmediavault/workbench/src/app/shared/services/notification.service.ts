import { Injectable } from '@angular/core';
import * as _ from 'lodash';
import { ToastrService } from 'ngx-toastr';
import { BehaviorSubject, Observable } from 'rxjs';

import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { Notification } from '~/app/shared/models/notification.model';

@Injectable({
  providedIn: 'root'
})
export class NotificationService {
  public notifications$: Observable<Notification[]>;

  private notificationsSource = new BehaviorSubject<Notification[]>([]);

  constructor(private toastr: ToastrService) {
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
      const fn = _.bind(this.toastr[notification.type], this.toastr);
      fn(notification.message, notification.title);
    }, 5);
  }

  add(notification: Notification): void {
    const notifications = this.notificationsSource.getValue();
    notifications.push(notification);
    this.notificationsSource.next(notifications);
  }

  cancel(id: number): void {
    window.clearTimeout(id);
  }

  remove(notification: Notification): void {
    const notifications = this.notificationsSource.getValue();
    _.remove(notifications, { id: notification.id });
    this.notificationsSource.next(notifications);
  }

  removeAll(): void {
    this.notificationsSource.next([]);
  }
}
