/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { Component, Input, OnDestroy } from '@angular/core';
import { MatSidenav } from '@angular/material/sidenav';
import { Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { EMPTY, of, Subscription } from 'rxjs';
import { catchError, delay, repeat } from 'rxjs/operators';
import { concatMap } from 'rxjs/operators';

import { translate } from '~/app/i18n.helper';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { Notification } from '~/app/shared/models/notification.model';
import { Permissions, Roles } from '~/app/shared/models/permissions.model';
import { AuthService } from '~/app/shared/services/auth.service';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { LocaleService } from '~/app/shared/services/locale.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import { SystemInformationService } from '~/app/shared/services/system-information.service';
import { UserStorageService } from '~/app/shared/services/user-storage.service';

@Component({
  selector: 'omv-top-bar',
  templateUrl: './top-bar.component.html',
  styleUrls: ['./top-bar.component.scss']
})
export class TopBarComponent implements OnDestroy {
  // eslint-disable-next-line @angular-eslint/no-input-rename
  @Input('navigationSidenav')
  navigationSidenav: MatSidenav;

  // eslint-disable-next-line @angular-eslint/no-input-rename
  @Input('notificationsSidenav')
  notificationsSidenav: MatSidenav;

  @BlockUI()
  blockUI: NgBlockUI;

  public icon = Icon;
  public currentLocale: string;
  public locales: Record<string, string> = {};
  public username: string;
  public hostname: string;
  public permissions: Permissions;
  public readonly roles = Roles;
  public numNotifications: undefined | number;

  private subscriptions = new Subscription();

  constructor(
    private router: Router,
    private authService: AuthService,
    private authSessionService: AuthSessionService,
    private rpcService: RpcService,
    private userStorageService: UserStorageService,
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private systemInformationService: SystemInformationService
  ) {
    this.currentLocale = LocaleService.getLocale();
    this.locales = LocaleService.getLocales();
    this.username = this.authSessionService.getUsername();
    this.permissions = this.authSessionService.getPermissions();
    this.subscriptions.add(
      this.notificationService.notifications$.subscribe((notifications: Notification[]) => {
        this.numNotifications = notifications.length ? notifications.length : undefined;
      })
    );
    this.subscriptions.add(
      this.systemInformationService.systemInfo$.subscribe((sysInfo) => {
        this.hostname = sysInfo.hostname;
      })
    );
  }

  ngOnDestroy(): void {
    this.subscriptions.unsubscribe();
  }

  onToggleNavigationSidenav() {
    this.navigationSidenav.toggle();
  }

  onToggleNotificationsSidenav() {
    this.notificationsSidenav.toggle();
  }

  onLogout() {
    this.showDialog(
      gettext('Logout'),
      gettext('Do you really want to logout?'),
      'confirmation',
      () => {
        this.blockUI.start(translate(gettext('Please wait ...')));
        this.authService.logout().subscribe();
      }
    );
  }

  onReboot() {
    this.showDialog(
      gettext('Reboot'),
      gettext('Do you really want to reboot the system?'),
      'confirmation-danger',
      () => {
        this.rpcService.request('System', 'reboot', { delay: 0 }).subscribe(() => {
          this.blockUI.start(
            translate(gettext('The system will reboot now. This may take some time ...'))
          );
          const subscription = of(true)
            .pipe(delay(5000))
            .pipe(
              concatMap(() => this.rpcService.request('System', 'noop')),
              catchError((error) => {
                // Do not show an error notification.
                error.preventDefault();
                // If we get a HTTP 401 Unauthorized status, then unblock UI.
                if (error.status === 401) {
                  subscription.unsubscribe();
                  this.blockUI.stop();
                }
                return EMPTY;
              }),
              delay(500),
              repeat()
            )
            .subscribe();
        });
      }
    );
  }

  onStandby() {
    this.showDialog(
      gettext('Standby'),
      gettext('Do you really want to put the system into standby?'),
      'confirmation-danger',
      () => {
        this.rpcService.request('System', 'standby', { delay: 0 }).subscribe(() => {
          this.router.navigate(['/standby']);
        });
      }
    );
  }

  onShutdown() {
    this.showDialog(
      gettext('Shutdown'),
      gettext('Do you really want to shutdown the system?'),
      'confirmation-danger',
      () => {
        this.rpcService.request('System', 'shutdown', { delay: 0 }).subscribe(() => {
          this.router.navigate(['/shutdown']);
        });
      }
    );
  }

  onLocale(locale) {
    // Update browser cookie and reload page.
    LocaleService.setLocale(locale);
    this.router.navigate(['/reload']);
  }

  onClearStateStorage() {
    // Clear browser cookies and reload page.
    this.showDialog(
      gettext('Reset UI to defaults'),
      gettext('Do you really want to reset the UI settings to their default values?'),
      'confirmation',
      () => {
        this.userStorageService.clear();
        this.router.navigate(['/reload']);
      }
    );
  }

  private showDialog(title: string, message: string, template: string, callback: () => void) {
    const dialogRef = this.dialogService.open(ModalDialogComponent, {
      data: { template, title, message }
    });
    dialogRef.afterClosed().subscribe((res) => {
      if (res) {
        callback();
      }
    });
  }
}
