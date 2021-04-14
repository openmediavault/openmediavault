import { Component } from '@angular/core';
import { Router } from '@angular/router';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { finalize } from 'rxjs/operators';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';
import { formatDeep, formatURI } from '~/app/functions.helper';
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
      const url = formatURI(this.config.url, formValues);
      this.router.navigate([url]);
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
        .subscribe(() => {
          // Display a notification?
          if (_.isString(request.successNotification)) {
            this.notificationService.show(NotificationType.success, request.successNotification);
          }
          // Navigate to a specified URL?
          if (_.isString(request.successUrl)) {
            const url = formatURI(request.successUrl, formValues);
            this.router.navigate([url]);
          }
        });
    }
  }
}
