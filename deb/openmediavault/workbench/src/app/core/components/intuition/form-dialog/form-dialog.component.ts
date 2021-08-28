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
import { Component, Inject, ViewChild } from '@angular/core';
import { MAT_DIALOG_DATA, MatDialogRef } from '@angular/material/dialog';
import { Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { finalize } from 'rxjs/operators';

import { FormComponent } from '~/app/core/components/intuition/form/form.component';
import {
  FormDialogButtonConfig,
  FormDialogConfig
} from '~/app/core/components/intuition/models/form-dialog-config.type';
import { format, formatDeep, formatURI } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

/**
 * A dialog that renders the specified form fields. On clicking the
 * 'Submit' button the form values are returned as an object, otherwise
 * 'false' on 'Cancel'.
 */
@Component({
  selector: 'omv-intuition-form-dialog',
  templateUrl: './form-dialog.component.html',
  styleUrls: ['./form-dialog.component.scss']
})
export class FormDialogComponent {
  @BlockUI()
  blockUI: NgBlockUI;

  @ViewChild(FormComponent, { static: true })
  form: FormComponent;

  // Internal
  public config: FormDialogConfig;

  constructor(
    private router: Router,
    private rpcService: RpcService,
    private notificationService: NotificationService,
    private matDialogRef: MatDialogRef<FormDialogComponent>,
    @Inject(MAT_DIALOG_DATA) data: FormDialogConfig
  ) {
    this.config = data;
    this.sanitizeConfig();
  }

  /**
   * Sets the values of the form fields.
   *
   * @param values The values to be set.
   */
  setFormValues(values: Array<any>) {
    // Set the value for each form field separately to prevent a
    // runtime error to be thrown when the values contains keys
    // for non-existing fields or when there is no form field for
    // a key.
    _.forEach(values, (value: any, key: string) => {
      const control = this.form.formGroup.get(key);
      if (!_.isNull(control)) {
        control.setValue(value);
      }
    });
  }

  /**
   * Get the values to be submitted. Ignore form fields where
   * 'submitValue=false' is set.
   *
   * @return Returns an object containing the form field values.
   */
  getFormValues(): Record<string, any> {
    const values = _.pickBy(this.form.formGroup.value, (value: any, key: string) => {
      const field = _.find(this.config.fields, { name: key });
      if (_.isUndefined(field)) {
        return true;
      }
      return _.defaultTo(field.submitValue, true);
    });
    return values;
  }

  onSubmit() {
    this.onButtonClick(this.config.buttons.submit);
  }

  onCancel() {
    this.onButtonClick(this.config.buttons.cancel);
  }

  protected sanitizeConfig() {
    _.defaultsDeep(this.config, {
      fields: [],
      buttons: {
        submit: {
          text: gettext('Save')
        },
        cancel: {
          text: gettext('Cancel'),
          dialogResult: false
        }
      }
    });
  }

  private onButtonClick(buttonConfig: FormDialogButtonConfig) {
    const values = this.getFormValues();
    const dialogResult = _.defaultTo(buttonConfig.dialogResult, values);
    switch (buttonConfig?.execute?.type) {
      case 'url':
        this.matDialogRef.close(dialogResult);
        this.router.navigate([buttonConfig.execute.url]);
        break;
      case 'request':
        const request = buttonConfig.execute.request;
        // Process the RPC parameters.
        let params = _.merge({}, values, formatDeep(request.params, values));
        if (_.get(request, 'intersectParams', false)) {
          const keys = _.intersection(_.keys(request.params), _.keys(values));
          params = _.pick(params, keys);
        }
        // Block UI and display the progress message.
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
            // Close dialog on success only, so the user can change the
            // data in case of an error without entering the form data
            // from scratch.
            this.matDialogRef.close(dialogResult);
            // Display a notification?
            if (_.isString(request.successNotification)) {
              this.notificationService.show(
                NotificationType.success,
                format(request.successNotification, values)
              );
            }
            // Navigate to a specified URL?
            if (_.isString(request.successUrl)) {
              const url = formatURI(request.successUrl, values);
              this.router.navigate([url]);
            }
          });
        break;
      default:
        this.matDialogRef.close(dialogResult);
        break;
    }
  }
}
