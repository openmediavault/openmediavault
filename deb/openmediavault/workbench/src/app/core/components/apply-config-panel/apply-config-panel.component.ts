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
import { ChangeDetectionStrategy, ChangeDetectorRef, Component } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';
import { finalize } from 'rxjs/operators';

import { Unsubscribe } from '~/app/decorators';
import { translate } from '~/app/i18n.helper';
import { AlertPanelButtonConfig } from '~/app/shared/components/alert-panel/alert-panel.component';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';

@Component({
  selector: 'omv-apply-config-panel',
  templateUrl: './apply-config-panel.component.html',
  styleUrls: ['./apply-config-panel.component.scss'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ApplyConfigPanelComponent {
  @Unsubscribe()
  private subscriptions: Subscription = new Subscription();

  public icon = Icon;
  public dirtyModules: Record<string, string> = {};
  public expanded = false;
  public buttons: AlertPanelButtonConfig[] = [];

  constructor(
    private blockUiService: BlockUiService,
    private cd: ChangeDetectorRef,
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private router: Router,
    private rpcService: RpcService,
    private systemInformationService: SystemInformationService
  ) {
    this.subscriptions.add(
      this.systemInformationService.systemInfo$.subscribe((res: SystemInformation) => {
        this.dirtyModules = _.get(res, 'dirtyModules', {});
        this.cd.markForCheck();
      })
    );
    this.buttons = [
      {
        tooltip: gettext('Show details'),
        icon: this.icon.details,
        click: () => (this.expanded = !this.expanded)
      },
      {
        tooltip: gettext('Undo'),
        icon: this.icon.undo,
        click: this.onUndoPendingChanges.bind(this)
      },
      {
        tooltip: gettext('Apply'),
        icon: this.icon.check,
        click: this.onApplyPendingChanges.bind(this)
      }
    ];
  }

  onApplyPendingChanges(): void {
    this.dialogService
      .open(ModalDialogComponent, {
        data: {
          template: 'confirmation-danger',
          title: gettext('Apply'),
          message: gettext('Do you really want to apply the configuration changes?')
        }
      })
      .afterClosed()
      .subscribe((res) => {
        if (res) {
          this.blockUiService.start(
            translate(gettext('Please wait, the configuration changes are being applied ...'))
          );
          this.rpcService
            .requestTask(
              'Config',
              'applyChangesBg',
              {
                modules: [],
                force: false
              },
              undefined,
              1000
            )
            .pipe(
              finalize(() => {
                this.blockUiService.stop();
              })
            )
            .subscribe(() => {
              this.notificationService.show(
                NotificationType.success,
                gettext('Applied the configuration changes.')
              );
            });
        }
      });
  }

  onUndoPendingChanges(): void {
    this.dialogService
      .open(ModalDialogComponent, {
        data: {
          template: 'confirmation-danger',
          title: gettext('Undo'),
          message: gettext('Do you really want to undo the configuration changes?')
        }
      })
      .afterClosed()
      .subscribe((res) => {
        if (res) {
          this.blockUiService.start(
            translate(gettext('Please wait, reverting configuration changes ...'))
          );
          this.rpcService
            .requestTask('Config', 'revertChangesBg', {
              filename: ''
            })
            .pipe(
              finalize(() => {
                this.blockUiService.stop();
              })
            )
            .subscribe(() => {
              this.notificationService.show(
                NotificationType.success,
                gettext('Reverted the configuration changes.')
              );
              this.router.navigate(['/reload']);
            });
        }
      });
  }
}
