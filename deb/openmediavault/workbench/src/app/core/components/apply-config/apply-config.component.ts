import { Component } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { finalize } from 'rxjs/operators';

import { translate } from '~/app/i18n.helper';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
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
    private matDialog: MatDialog,
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
          .requestTask('Config', 'applyChangesBg', {
            modules: [],
            force: false
          })
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
    const dialogRef = this.matDialog.open(ModalDialogComponent, {
      data: { template: 'confirmation', title, message }
    });
    dialogRef.afterClosed().subscribe((res) => {
      if (res) {
        callback();
      }
    });
  }
}
