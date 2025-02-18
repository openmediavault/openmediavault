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
import { Component, Inject, ViewChild } from '@angular/core';
import { MAT_DIALOG_DATA, MatDialogRef } from '@angular/material/dialog';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { finalize } from 'rxjs/operators';

import { FormComponent } from '~/app/core/components/intuition/form/form.component';
import { FormFieldName } from '~/app/core/components/intuition/models/form.type';
import { FormValues } from '~/app/core/components/intuition/models/form.type';
import {
  FormDialogButtonConfig,
  FormDialogConfig
} from '~/app/core/components/intuition/models/form-dialog-config.type';
import { format, formatDeep, isFormatable } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import { TaskDialogComponent } from '~/app/shared/components/task-dialog/task-dialog.component';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { DialogService } from '~/app/shared/services/dialog.service';
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
  @ViewChild(FormComponent, { static: true })
  form: FormComponent;

  // Internal
  public config: FormDialogConfig;

  constructor(
    private blockUiService: BlockUiService,
    private dialogService: DialogService,
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
   * Sets the form values.
   *
   * @param values The values to be set.
   * @param markAsPristine Mark the form as pristine. Defaults to `true`.
   */
  setFormValues(values: FormValues, markAsPristine = true): void {
    this.form.formGroup.patchValue(values);
    if (markAsPristine) {
      this.form.formGroup.markAsPristine();
    }
  }

  /**
   * Get the values to be submitted. Ignore form fields where
   * 'submitValue=false' is set.
   *
   * @return Returns an object containing the form field values.
   */
  getFormValues(): FormValues {
    const values: FormValues = _.pickBy(
      this.form.formGroup.getRawValue(),
      (value: any, key: FormFieldName) => {
        const field = _.find(this.config.fields, { name: key });
        if (_.isUndefined(field)) {
          return true;
        }
        return _.defaultTo(field.submitValue, true);
      }
    );
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
    const values: FormValues = this.getFormValues();
    const dialogResult = _.defaultTo(buttonConfig.dialogResult, values);
    switch (buttonConfig?.execute?.type) {
      case 'url':
        this.matDialogRef.close(dialogResult);
        const url = format(buttonConfig.execute.url, values);
        this.router.navigateByUrl(url);
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
          this.blockUiService.start(translate(request.progressMessage));
        }
        this.rpcService[request.task ? 'requestTask' : 'request'](
          request.service,
          request.method,
          params
        )
          .pipe(
            finalize(() => {
              if (_.isString(request.progressMessage)) {
                this.blockUiService.stop();
              }
            })
          )
          .subscribe(() => {
            // Close dialog on success only, so the user can change the
            // data in case of an error without entering the form data
            // again from scratch.
            this.matDialogRef.close(dialogResult);
            // Display a notification?
            if (_.isString(request.successNotification)) {
              this.notificationService.show(
                NotificationType.success,
                undefined,
                format(request.successNotification, values)
              );
            }
            // Navigate to a specified URL?
            if (_.isString(request.successUrl)) {
              const successUrl = format(request.successUrl, values);
              this.router.navigateByUrl(successUrl);
            }
          });
        break;
      case 'taskDialog':
        const taskDialog = _.cloneDeep(buttonConfig.execute.taskDialog);
        // Process tokenized configuration properties.
        _.forEach(['request.params'], (path) => {
          const value = _.get(taskDialog.config, path);
          if (isFormatable(value)) {
            _.set(taskDialog.config, path, formatDeep(value, values));
          }
        });
        const dialog = this.dialogService.open(TaskDialogComponent, {
          width: _.get(taskDialog.config, 'width', '75%'),
          data: _.omit(taskDialog.config, ['width'])
        });
        // Navigate to the specified URL if pressed button returns `true`.
        dialog.afterClosed().subscribe((res) => {
          if (res) {
            // Close dialog on success only, so the user can change the
            // data in case of an error without entering the form data
            // again from scratch.
            this.matDialogRef.close(dialogResult);
            // Navigate to a specified URL?
            if (_.isString(taskDialog.successUrl)) {
              const successUrl = format(taskDialog.successUrl, values);
              this.router.navigateByUrl(successUrl);
            }
          }
        });
        break;
      default:
        this.matDialogRef.close(dialogResult);
        break;
    }
  }
}
