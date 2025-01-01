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
import { Component, EventEmitter, Output } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import { EMPTY, interval, Subscription } from 'rxjs';
import { catchError, switchMap, take } from 'rxjs/operators';

import { Unsubscribe } from '~/app/decorators';
import { format } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { Permissions, Roles } from '~/app/shared/models/permissions.model';
import { AuthService } from '~/app/shared/services/auth.service';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { LocaleService } from '~/app/shared/services/locale.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { PrefersColorSchemeService } from '~/app/shared/services/prefers-color-scheme.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

@Component({
  selector: 'omv-top-bar',
  templateUrl: './top-bar.component.html',
  styleUrls: ['./top-bar.component.scss']
})
export class TopBarComponent {
  @Output()
  readonly navigationToggleChange = new EventEmitter();

  @Output()
  readonly notificationToggleChange = new EventEmitter();

  @Unsubscribe()
  private subscriptions = new Subscription();

  public icon = Icon;
  public currentLocale: string;
  public locales: Record<string, string> = {};
  public username: string;
  public hostname: string;
  public permissions: Permissions;
  public readonly roles = Roles;
  public numNotifications: undefined | number;
  public darkModeEnabled: boolean;
  public loggedInAs: string;

  constructor(
    private blockUiService: BlockUiService,
    private authService: AuthService,
    private authSessionService: AuthSessionService,
    private prefersColorSchemeService: PrefersColorSchemeService,
    private router: Router,
    private rpcService: RpcService,
    private userLocalStorageService: UserLocalStorageService,
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private systemInformationService: SystemInformationService
  ) {
    this.currentLocale = LocaleService.getCurrentLocale();
    this.locales = LocaleService.getSupportedLocales();
    this.username = this.authSessionService.getUsername();
    this.loggedInAs = gettext(
      format('Logged in as <strong>{{ username }}</strong>', { username: this.username })
    );
    this.permissions = this.authSessionService.getPermissions();
    this.darkModeEnabled = this.prefersColorSchemeService.current === 'dark';
    this.subscriptions.add(
      this.notificationService.notifications$.subscribe(() => this.updateNumNotifications())
    );
    this.subscriptions.add(
      this.systemInformationService.systemInfo$.subscribe((sysInfo: SystemInformation) => {
        this.hostname = sysInfo.hostname;
        this.updateNumNotifications();
      })
    );
  }

  onToggleNavigation(event: Event): void {
    event.stopPropagation();
    this.navigationToggleChange.emit();
  }

  onToggleNotification(event: Event): void {
    event.stopPropagation();
    this.notificationToggleChange.emit();
  }

  onLogout(): void {
    this.showDialog(
      gettext('Logout'),
      gettext('Do you really want to logout?'),
      'confirmation',
      () => {
        this.blockUiService.start(translate(gettext('Please wait ...')));
        this.authService.logout().subscribe();
      }
    );
  }

  onReboot(): void {
    this.showDialog(
      gettext('Reboot'),
      gettext('Do you really want to reboot the system?'),
      'confirmation-critical',
      () => {
        this.rpcService.request('System', 'reboot', { delay: 0 }).subscribe(() => {
          this.blockUiService.start(
            translate(gettext('The system will reboot now. This may take some time ...'))
          );
          const subscription = interval(5000)
            .pipe(
              switchMap(() =>
                this.rpcService.request('System', 'noop').pipe(
                  catchError((error) => {
                    // Do not show an error notification.
                    error.preventDefault?.();
                    // Check if we got a 'HTTP 401 Unauthorized status'.
                    // In that case the request was successful, but
                    // authentication failed => this means the system is
                    // up again.
                    if (error.status === 401) {
                      subscription.unsubscribe();
                      this.blockUiService.stop();
                    }
                    return EMPTY;
                  })
                )
              )
            )
            .subscribe();
        });
      }
    );
  }

  onStandby(): void {
    this.showDialog(
      gettext('Standby'),
      gettext('Do you really want to put the system into standby?'),
      'confirmation-critical',
      () => {
        this.rpcService.request('System', 'standby', { delay: 0 }).subscribe(() => {
          this.router.navigate(['/standby']);
        });
      }
    );
  }

  onShutdown(): void {
    this.showDialog(
      gettext('Shutdown'),
      gettext('Do you really want to shutdown the system?'),
      'confirmation-critical',
      () => {
        this.rpcService.request('System', 'shutdown', { delay: 0 }).subscribe(() => {
          this.router.navigate(['/shutdown']);
        });
      }
    );
  }

  onLocale(locale): void {
    // Update browser cookie and reload page.
    LocaleService.setCurrentLocale(locale);
    this.router.navigate(['/reload']);
  }

  onClearStateStorage(): void {
    // Clear browser cookies and reload page.
    this.showDialog(
      gettext('Reset UI to defaults'),
      gettext('Do you really want to reset the UI settings to their default values?'),
      'confirmation',
      () => {
        this.userLocalStorageService.clear();
        this.router.navigate(['/reload']);
      }
    );
  }

  onToggleDarkMode(): void {
    this.prefersColorSchemeService.toggle();
    this.darkModeEnabled = !this.darkModeEnabled;
  }

  private showDialog(title: string, message: string, template: string, callback: () => void): void {
    const dialogRef = this.dialogService.open(ModalDialogComponent, {
      data: { template, title, message }
    });
    dialogRef.afterClosed().subscribe((res) => {
      if (res) {
        callback();
      }
    });
  }

  private updateNumNotifications(): void {
    this.systemInformationService.systemInfo$
      .pipe(take(1))
      .subscribe((sysInfo: SystemInformation) => {
        let numNotifications: number = this.notificationService.getAll().length;
        if (sysInfo.rebootRequired) {
          numNotifications += 1;
        }
        if (sysInfo.availablePkgUpdates > 0) {
          numNotifications += 1;
        }
        this.numNotifications = numNotifications;
      });
  }
}
