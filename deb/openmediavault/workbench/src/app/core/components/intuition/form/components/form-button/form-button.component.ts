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
import { Component } from '@angular/core';
import { Router } from '@angular/router';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { finalize } from 'rxjs/operators';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { format, formatDeep } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-form-button',
  templateUrl: './form-button.component.html',
  styleUrls: ['./form-button.component.scss']
})
export class FormButtonComponent extends AbstractFormFieldComponent {
  @BlockUI()
  blockUI: NgBlockUI;

  constructor(
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
      const request = this.config.request;
      const params = formatDeep(request.params, formValues);
      if (_.isString(request.progressMessage)) {
        this.blockUI.start(translate(request.progressMessage));
      }
      this.rpcService[request.task ? 'requestTask' : 'request'](
        request.service,
        request.method,
        params
      )
        .pipe(
          finalize(() => {
            if (_.isString(request.progressMessage)) {
              this.blockUI.stop();
            }
          })
        )
        .subscribe((res: any) => {
          // Display a notification?
          if (_.isString(request.successNotification)) {
            this.notificationService.show(
              NotificationType.success,
              undefined,
              format(request.successNotification, _.merge({ _response: res }, formValues))
            );
          }
          // Navigate to a specified URL?
          if (_.isString(request.successUrl)) {
            const url = format(
              request.successUrl,
              _.merge(
                {
                  _response: res
                },
                formValues
              )
            );
            this.router.navigateByUrl(url);
          }
        });
    }
  }
}
