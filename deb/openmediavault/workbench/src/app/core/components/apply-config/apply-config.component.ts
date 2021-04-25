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
import { Component } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { finalize } from 'rxjs/operators';

import { translate } from '~/app/i18n.helper';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { DialogService } from '~/app/shared/services/dialog.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-apply-config',
  templateUrl: './apply-config.component.html',
  styleUrls: ['./apply-config.component.scss']
})
export class ApplyConfigComponent {
  @BlockUI()
  blockUI: NgBlockUI;

  public icon = Icon;

  constructor(
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private rpcService: RpcService
  ) {}

  onApplyPendingChanges(): void {
    this.showDialog(
      gettext('Apply'),
      gettext('Do you really want to apply the configuration changes?'),
      () => {
        this.blockUI.start(
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
              this.blockUI.stop();
            })
          )
          .subscribe(() => {
            this.notificationService.show(
              NotificationType.success,
              gettext('Applied the configuration changes.')
            );
          });
      }
    );
  }

  onUndoPendingChanges(): void {
    this.showDialog(
      gettext('Undo'),
      gettext('Do you really want to undo the configuration changes?'),
      () => {
        this.blockUI.start(translate(gettext('Please wait, reverting configuration changes ...')));
        this.rpcService
          .requestTask('Config', 'revertChangesBg', {
            filename: ''
          })
          .pipe(
            finalize(() => {
              this.blockUI.stop();
            })
          )
          .subscribe(() => {
            this.notificationService.show(
              NotificationType.success,
              gettext('Reverted the configuration changes.')
            );
          });
      }
    );
  }

  private showDialog(title: string, message: string, callback: () => void): void {
    const dialogRef = this.dialogService.open(ModalDialogComponent, {
      data: { template: 'confirmation', title, message }
    });
    dialogRef.afterClosed().subscribe((res) => {
      if (res) {
        callback();
      }
    });
  }
}
