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
import { Component, ViewEncapsulation } from '@angular/core';
import { AbstractControl } from '@angular/forms';
import { Router } from '@angular/router';
import * as _ from 'lodash';
import { finalize } from 'rxjs/operators';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { format, formatDeep } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-form-button',
  templateUrl: './form-button.component.html',
  styleUrls: ['./form-button.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class FormButtonComponent extends AbstractFormFieldComponent {
  constructor(
    private blockUiService: BlockUiService,
    private clipboardService: ClipboardService,
    private notificationService: NotificationService,
    private rpcService: RpcService,
    private router: Router
  ) {
    super();
  }

  onClick() {
    const formValues = this.formGroup.getRawValue();
    if (_.isFunction(this.config.click)) {
      // Call the callback function.
      this.config.click();
    } else if (_.isString(this.config.url)) {
      // Navigate to the specified URL.
      const url = format(this.config.url, formValues);
      this.router.navigateByUrl(url);
    } else if (_.isPlainObject(this.config.request)) {
      // Execute the specified request.
      const control: AbstractControl = this.formGroup.get(this.config.name);
      const request = this.config.request;
      const params = formatDeep(request.params, formValues);
      if (_.isString(request.progressMessage)) {
        this.blockUiService.start(translate(request.progressMessage));
      }
      control.disable();
      this.rpcService[request.task ? 'requestTask' : 'request'](
        request.service,
        request.method,
        params
      )
        .pipe(
          finalize(() => {
            control.enable();
            if (_.isString(request.progressMessage)) {
              this.blockUiService.stop();
            }
          })
        )
        .subscribe((res: any) => {
          const data: Record<any, any> = _.merge({ _response: res }, formValues);
          // Display a notification?
          if (_.isString(request.successNotification)) {
            const successNotification: string = format(request.successNotification, data);
            this.notificationService.show(NotificationType.success, undefined, successNotification);
          }
          // Copy the response to the clipboard?
          if (_.isString(request.successCopyToClipboard)) {
            const successCopyToClipboard: string = format(request.successCopyToClipboard, data);
            this.clipboardService.copy(successCopyToClipboard);
          }
          // Navigate to a specified URL?
          if (_.isString(request.successUrl)) {
            const successUrl: string = format(request.successUrl, data);
            this.router.navigateByUrl(successUrl);
          }
        });
    }
  }
}
