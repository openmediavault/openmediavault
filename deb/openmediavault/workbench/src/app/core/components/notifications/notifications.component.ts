import { Component, OnInit } from '@angular/core';
import * as _ from 'lodash';

import { Icon } from '~/app/shared/enum/icon.enum';
import { Notification } from '~/app/shared/models/notification.model';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { NotificationService } from '~/app/shared/services/notification.service';

@Component({
  selector: 'omv-notifications',
  templateUrl: './notifications.component.html',
  styleUrls: ['./notifications.component.scss']
})
export class NotificationsComponent implements OnInit {
  public icon = Icon;
  public notifications: Notification[];

  constructor(
    private clipboardService: ClipboardService,
    private notificationService: NotificationService
  ) {}

  ngOnInit(): void {
    this.notificationService.notifications$.subscribe((notifications: Notification[]) => {
      const data = _.clone(notifications);
      _.reverse(data);
      this.notifications = data;
    });
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
