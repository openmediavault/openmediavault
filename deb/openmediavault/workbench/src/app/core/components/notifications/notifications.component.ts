/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import { Component, OnDestroy } from '@angular/core';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';
import { delay, map } from 'rxjs/operators';

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
  public notifications: Notification[] = [];

  private subscription: Subscription;

  constructor(
    private clipboardService: ClipboardService,
    private notificationService: NotificationService
  ) {
    this.subscription = this.notificationService.notifications$
      .pipe(
        map((notifications) => _.reverse(_.clone(notifications))),
        delay(0)
      )
      .subscribe((notifications: Notification[]) => {
        this.notifications = notifications;
      });
  }

  ngOnDestroy(): void {
    this.subscription.unsubscribe();
  }

  onRemoveNotification(notification: Notification): void {
    this.notificationService.remove(notification);
  }

  onRemoveAllNotifications(): void {
    this.notificationService.removeAll();
  }

  onCopyNotification(notification: Notification): void {
    this.clipboardService.copy(`${notification.message}\n\n${notification.traceback}`);
  }
}
