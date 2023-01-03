/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2023 Volker Theile
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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { MatSidenav } from '@angular/material/sidenav';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { Subscription, timer } from 'rxjs';
import { take, takeUntil } from 'rxjs/operators';

import { TaskDialogComponent } from '~/app/shared/components/task-dialog/task-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { Notification } from '~/app/shared/models/notification.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';
import { RunningTasks, TaskRunnerService } from '~/app/shared/services/task-runner.service';

@Component({
  selector: 'omv-notification-bar',
  templateUrl: './notification-bar.component.html',
  styleUrls: ['./notification-bar.component.scss']
})
export class NotificationBarComponent implements OnInit, OnDestroy {
  @Input()
  sidenav?: MatSidenav;

  public icon = Icon;
  public notifications: Notification[] = [];
  public tasks: RunningTasks;
  public dismissibleNotifications = false;

  private subscriptions: Subscription = new Subscription();

  constructor(
    private authSessionService: AuthSessionService,
    private clipboardService: ClipboardService,
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private rpcService: RpcService,
    private systemInformationService: SystemInformationService,
    private taskRunnerService: TaskRunnerService
  ) {}

  ngOnInit(): void {
    this.subscriptions.add(
      this.notificationService.notifications$.subscribe(() => this.updateNotifications())
    );
    this.subscriptions.add(
      this.systemInformationService.systemInfo$.subscribe(() => this.updateNotifications())
    );
    if (this.authSessionService.hasAdminRole()) {
      this.subscriptions.add(
        this.sidenav.openedStart.subscribe(() => {
          timer(0, 5000)
            .pipe(takeUntil(this.sidenav.closedStart))
            .subscribe(() => this.loadTasks());
        })
      );
    }
  }

  ngOnDestroy(): void {
    this.subscriptions.unsubscribe();
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

  onStopTask(filename: string): void {
    this.rpcService
      .request('Exec', 'stop', {
        filename
      })
      .subscribe({
        complete: () => this.loadTasks()
      });
  }

  onAttachTask(filename: string): void {
    this.sidenav.close();
    this.dialogService.open(TaskDialogComponent, {
      width: '75%',
      data: {
        title: gettext('Background task'),
        startOnInit: true,
        request: {
          service: 'Exec',
          method: 'attach',
          params: {
            filename
          }
        }
      }
    });
  }

  private loadTasks(): void {
    this.taskRunnerService.enumerate().subscribe((tasks: RunningTasks) => {
      this.tasks = tasks;
    });
  }

  private updateNotifications(): void {
    this.systemInformationService.systemInfo$
      .pipe(take(1))
      .subscribe((sysInfo: SystemInformation) => {
        // Make a deep copy of the notifications and reverse the order.
        const notifications = _.reverse(_.clone(this.notificationService.list()));
        // Append additional notifications.
        if (sysInfo.rebootRequired) {
          const notification: Notification = new Notification(
            NotificationType.info,
            gettext('System restart required')
          );
          notification.dismissible = false;
          notifications.unshift(notification);
        }
        if (sysInfo.availablePkgUpdates > 0) {
          const notification: Notification = new Notification(
            NotificationType.info,
            gettext('Updates available')
          );
          notification.dismissible = false;
          notifications.unshift(notification);
        }
        this.dismissibleNotifications = _.filter(notifications, ['dismissible', true]).length > 0;
        this.notifications = notifications;
      });
  }
}
