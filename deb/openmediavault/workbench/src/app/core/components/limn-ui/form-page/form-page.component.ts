import { HttpErrorResponse } from '@angular/common/http';
import { AfterViewInit, Component, Inject, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { ActivatedRoute, Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { EMPTY, Subscription } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { AbstractPageComponent } from '~/app/core/components/limn-ui/abstract-page-component';
import { FormComponent } from '~/app/core/components/limn-ui/form/form.component';
import {
  flattenFormFieldConfig,
  setupConfObjUuidFields
} from '~/app/core/components/limn-ui/functions.helper';
import { FormFieldConfig } from '~/app/core/components/limn-ui/models/form-field-config.type';
import {
  FormPageButtonConfig,
  FormPageConfig
} from '~/app/core/components/limn-ui/models/form-page-config.type';
import { format, formatDeep, formatURI, isFormatable, toBoolean } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { RpcObjectResponse } from '~/app/shared/models/rpc.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { ConstraintService } from '~/app/shared/services/constraint.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

/**
 * This component will render a page containing a form with the
 * configured form fields. By default this page contains a 'Save'
 * and 'Cancel' button. The 'Save' button is enabled when the form
 * is dirty and the form validation was successfully.
 */
@Component({
  selector: 'omv-limn-form-page',
  templateUrl: './form-page.component.html',
  styleUrls: ['./form-page.component.scss']
})
export class FormPageComponent
  extends AbstractPageComponent<FormPageConfig>
  implements AfterViewInit, OnInit, OnDestroy {
  @BlockUI()
  blockUI: NgBlockUI;

  @ViewChild(FormComponent, { static: true })
  form: FormComponent;

  // Internal
  public editing = false;
  public loading = false;
  public error: HttpErrorResponse;

  private subscriptions = new Subscription();

  constructor(
    @Inject(ActivatedRoute) activatedRoute: ActivatedRoute,
    @Inject(AuthSessionService) authSessionService: AuthSessionService,
    private router: Router,
    private rpcService: RpcService,
    private matDialog: MatDialog,
    private notificationService: NotificationService
  ) {
    super(activatedRoute, authSessionService);
    // Set the form mode to 'Create' (default) or 'Edit'.
    // This depends on the component configuration that is done via the
    // router config.
    // Examples:
    // {
    //   path: 'hdparm/create',
    //   component: DiskFormPageComponent,
    //   data: { title: gettext('Create'), editing: false }
    // }
    // {
    //   path: 'hdparm/edit/:devicefile',
    //   component: DiskFormPageComponent,
    //   data: { title: gettext('Edit'), editing: true }
    // }
    this.editing = _.get(this.routeConfig, 'data.editing', false);
  }

  ngOnInit(): void {
    super.ngOnInit();
    // Flatten all form field configurations into an array to be able to
    // iterate over them easily.
    const allFields = flattenFormFieldConfig(this.config.fields);
    // Process the 'disabled' attribute in all form field configurations.
    _.forEach(allFields, (fieldConfig: FormFieldConfig) => {
      if (_.has(fieldConfig, 'disabled') && _.isString(fieldConfig.disabled)) {
        fieldConfig.disabled = toBoolean(format(String(fieldConfig.disabled), this.pageContext));
      }
    });
    // Process the 'required' validator in all form field configurations.
    _.forEach(allFields, (fieldConfig: FormFieldConfig) => {
      if (
        _.has(fieldConfig, 'validators.required') &&
        _.isString(fieldConfig.validators.required)
      ) {
        fieldConfig.validators.required = toBoolean(
          format(String(fieldConfig.validators.required), this.pageContext)
        );
      }
    });
  }

  ngOnDestroy(): void {
    this.subscriptions.unsubscribe();
  }

  ngAfterViewInit(): void {
    // Process all specified constraints per button.
    if (_.some(this.config.buttons, (button) => _.isPlainObject(button.enabledConstraint))) {
      this.subscriptions.add(
        this.form.formGroup.valueChanges.subscribe((values: Record<any, any>) => {
          _.forEach(this.config.buttons, (button) => {
            if (_.isPlainObject(button.enabledConstraint)) {
              button.disabled = !ConstraintService.test(button.enabledConstraint, values);
            }
          });
        })
      );
    }
  }

  loadData(): void {
    const request = this.config.request;
    if (_.isString(request?.service) && _.isPlainObject(request?.get)) {
      this.loading = true;
      // noinspection DuplicatedCode
      this.rpcService[request.get.task ? 'requestTask' : 'request'](
        request.service,
        request.get.method,
        request.get.params
      )
        .pipe(
          catchError((error) => {
            this.error = error;
            return EMPTY;
          }),
          finalize(() => {
            this.loading = false;
          })
        )
        .subscribe((res: RpcObjectResponse) => {
          this.onLoadData(res);
        });
    }
  }

  onLoadData(res: RpcObjectResponse): void {
    const request = this.config.request;
    // Transform the request response?
    if (_.isPlainObject(request?.get?.transform)) {
      res = RpcObjectResponse.transform(res, request.get.transform);
    }
    // Filter the request response?
    if (_.isPlainObject(request?.get?.filter)) {
      const filterConfig = request.get.filter;
      res = RpcObjectResponse.filter(res, filterConfig.props, filterConfig.mode);
    }
    // Update the form field values.
    this.setFormValues(res);
  }

  /**
   * Sets the form values.
   */
  setFormValues(values: Record<string, any>): void {
    this.form.formGroup.patchValue(values);
  }

  /**
   * Get the values to be submitted. Ignore form fields where
   * 'submitValue=false' is set.
   *
   * @return Returns an object containing the form field values.
   */
  getFormValues(): Record<string, any> {
    const allFields = flattenFormFieldConfig(this.config.fields);
    const values = _.pickBy(this.form.formGroup.getRawValue(), (value, key) => {
      const field = _.find(allFields, { name: key });
      if (_.isUndefined(field)) {
        return true;
      }
      return _.defaultTo(field.submitValue, true);
    });
    return values;
  }

  onButtonClick(buttonConfig: FormPageButtonConfig) {
    let values = this.getFormValues();
    // Closure that handles the button action.
    const doButtonActionFn = () => {
      switch (buttonConfig?.execute?.type) {
        case 'click':
          if (_.isFunction(buttonConfig.execute.click)) {
            // Call the callback function.
            buttonConfig.execute.click(buttonConfig, values);
          }
          break;
        case 'url':
          if (_.isString(buttonConfig.execute.url)) {
            // Navigate to the specified URL.
            const url = formatURI(buttonConfig.execute.url, _.merge({}, values, this.pageContext));
            this.router.navigate([url]);
          }
          break;
        case 'request':
          if (_.isPlainObject(buttonConfig.execute.request)) {
            // Execute the specified request.
            const request = buttonConfig.execute.request;
            if (_.isString(request.progressMessage)) {
              this.blockUI.start(translate(request.progressMessage));
            }
            this.rpcService[request.task ? 'requestTask' : 'request'](
              request.service,
              request.method,
              formatDeep(request.params, this.pageContext)
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
                  this.notificationService.show(
                    NotificationType.success,
                    format(request.successNotification, _.merge({}, this.pageContext, values))
                  );
                }
                // Navigate to a specified URL?
                if (_.isString(request.successUrl)) {
                  const url = formatURI(request.successUrl, _.merge({}, values, this.pageContext));
                  this.router.navigate([url]);
                }
              });
          }
          break;
      }
    };
    // Closure that handles the button pre-action.
    const doPreButtonActionFn = () => {
      // Must the user confirm the action?
      if (_.isPlainObject(buttonConfig.confirmationDialogConfig)) {
        const data = _.cloneDeep(buttonConfig.confirmationDialogConfig);
        if (_.isString(data.message)) {
          data.message = format(data.message, values);
        }
        const dialogRef = this.matDialog.open(ModalDialogComponent, { data });
        dialogRef.afterClosed().subscribe((res: any) => {
          if (true === res) {
            doButtonActionFn();
          }
        });
      } else {
        doButtonActionFn();
      }
    };
    // Submit the form values?
    if (buttonConfig.template === 'submit') {
      const request = this.config?.request;
      if (
        _.isPlainObject(request) &&
        _.isString(request.service) &&
        _.isPlainObject(request.post)
      ) {
        const doRpcRequestFn = () => {
          // Process the RPC parameters.
          if (_.isPlainObject(request.post.params)) {
            const params = formatDeep(request.post.params, _.merge(this.pageContext, values));
            let tmp = _.merge({}, values, params);
            if (_.get(request.post, 'intersectParams', false)) {
              const keys = _.intersection(_.keys(request.post.params), _.keys(values));
              tmp = _.pick(tmp, keys);
            }
            values = tmp;
          }
          if (_.isString(request.post.progressMessage)) {
            this.blockUI.start(translate(request.post.progressMessage));
          }
          this.rpcService[request.post.task ? 'requestTask' : 'request'](
            request.service,
            request.post.method,
            values
          )
            .pipe(
              finalize(() => {
                if (_.isString(request.post.progressMessage)) {
                  this.blockUI.stop();
                }
              })
            )
            .subscribe(() => {
              const notificationTitle = _.get(this.routeConfig, 'data.notificationTitle');
              if (!_.isEmpty(notificationTitle)) {
                this.notificationService.show(
                  NotificationType.success,
                  format(notificationTitle, _.merge({}, this.pageContext, values))
                );
              }
              doPreButtonActionFn();
            });
        };
        // Has the user to confirm the RPC request?
        if (_.isPlainObject(request.post.confirmationDialogConfig)) {
          const data = _.cloneDeep(request.post.confirmationDialogConfig);
          if (_.isString(data.message)) {
            data.message = format(data.message, values);
          }
          const dialogRef = this.matDialog.open(ModalDialogComponent, { data });
          dialogRef.afterClosed().subscribe((res: any) => {
            if (true === res) {
              // Execute the RPC request.
              doRpcRequestFn();
            }
          });
        } else {
          doRpcRequestFn();
        }
      } else {
        doPreButtonActionFn();
      }
    } else {
      doPreButtonActionFn();
    }
  }

  protected sanitizeConfig() {
    _.defaultsDeep(this.config, {
      buttonAlign: 'left',
      buttons: []
    });
    // Populate the datamodel identifier field. This must be done here
    // in addition to the `FormComponent`, since the form has not yet
    // been initialized at this point in time and the fields have
    // therefore not yet been setup.
    setupConfObjUuidFields(this.config.fields);
    // Set the default values of the buttons.
    _.forEach(this.config.buttons, (button) => {
      _.defaultsDeep(button, {
        disabled: false
      });
      const template = _.get(button, 'template');
      switch (template) {
        case 'back':
          _.defaultsDeep(button, {
            text: gettext('Back')
          });
          break;
        case 'cancel':
          _.defaultsDeep(button, {
            text: gettext('Cancel')
          });
          break;
        case 'submit':
          _.defaultsDeep(button, {
            text: gettext('Save')
          });
          break;
      }
    });
    // Map icon from 'foo' to 'mdi:foo' if necessary.
    this.config.icon = _.get(Icon, this.config.icon, this.config.icon);
  }

  protected onRouteParams() {
    const allFields = flattenFormFieldConfig(this.config.fields);
    // Format tokenized configuration properties.
    this.formatConfig(['title', 'subTitle', 'request.get.method', 'request.get.params']);
    // Load the content if form page is in 'editing' mode.
    if (this.editing) {
      this.loadData();
    }
    // Inject route configuration and parameters into various form field
    // configuration properties.
    _.forEach(allFields, (fieldConfig: FormFieldConfig) => {
      _.forEach(['store.proxy', 'store.filters', 'value', 'request.params'], (path) => {
        const value = _.get(fieldConfig, path);
        if (isFormatable(value)) {
          _.set(fieldConfig, path, formatDeep(value, this.pageContext));
        }
      });
    });
  }
}
