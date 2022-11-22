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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { MatSidenav } from '@angular/material/sidenav';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { Subscription, timer } from 'rxjs';
import { delay, map, takeUntil } from 'rxjs/operators';

import { TaskDialogComponent } from '~/app/shared/components/task-dialog/task-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { Notification } from '~/app/shared/models/notification.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';
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

  private subscriptions: Subscription = new Subscription();

  constructor(
    private authSessionService: AuthSessionService,
    private clipboardService: ClipboardService,
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private rpcService: RpcService,
    private taskRunnerService: TaskRunnerService
  ) {}

  ngOnInit(): void {
    this.subscriptions.add(
      this.notificationService.notifications$
        .pipe(
          map((notifications) => _.reverse(_.clone(notifications))),
          delay(0)
        )
        .subscribe((notifications: Notification[]) => {
          this.notifications = notifications;
        })
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
}
