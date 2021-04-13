import { Component, OnDestroy } from '@angular/core';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';

import { Icon } from '~/app/shared/enum/icon.enum';
import { Notification } from '~/app/shared/models/notification.model';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { NotificationService } from '~/app/shared/services/notification.service';

@Component({
  selector: 'omv-notifications',
  templateUrl: './notifications.component.html',
  styleUrls: ['./notifications.component.scss']
})
export class NotificationsComponent implements OnDestroy {
  public icon = Icon;
  public notifications: Notification[];

  private subscription: Subscription;

  constructor(
    private clipboardService: ClipboardService,
    private notificationService: NotificationService
  ) {
    this.subscription = this.notificationService.notifications$.subscribe(
      (notifications: Notification[]) => {
        const data = _.clone(notifications);
        _.reverse(data);
        this.notifications = data;
      }
    );
  }

  ngOnDestroy(): void {
    this.subscription.unsubscribe();
  }

  onRemoveNotification(notification: Notification) {
    this.notificationService.remove(notification);
  }

  onRemoveAllNotifications() {
    this.notificationService.removeAll();
  }

  onCopyNotification(notification: Notification) {
    this.clipboardService.copy(`${notification.message}\n\n${notification.traceback}`);
  }
}
