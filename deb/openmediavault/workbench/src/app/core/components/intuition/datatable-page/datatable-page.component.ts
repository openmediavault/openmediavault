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
import { ActivatedRoute, Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { concat } from 'rxjs';
import { finalize } from 'rxjs/operators';

import {
  AbstractPageComponent,
  PageContext
} from '~/app/core/components/intuition/abstract-page-component';
import { FormDialogComponent } from '~/app/core/components/intuition/form-dialog/form-dialog.component';
import { DatatablePageActionConfig } from '~/app/core/components/intuition/models/datatable-page-action-config.type';
import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { DatatablePageButtonConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { FormFieldConfig } from '~/app/core/components/intuition/models/form-field-config.type';
import { format, formatDeep, isFormatable } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import {
  DatatableComponent,
  DataTableLoadParams
} from '~/app/shared/components/datatable/datatable.component';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { TaskDialogComponent } from '~/app/shared/components/task-dialog/task-dialog.component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { DatatableAction } from '~/app/shared/models/datatable-action.type';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';
import { RpcListResponse } from '~/app/shared/models/rpc.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { DataStoreService } from '~/app/shared/services/data-store.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-intuition-datatable-page',
  templateUrl: './datatable-page.component.html',
  styleUrls: ['./datatable-page.component.scss']
})
export class DatatablePageComponent extends AbstractPageComponent<DatatablePageConfig> {
  @ViewChild('table', { static: true })
  table: DatatableComponent;

  public loading = false;
  public count = 0;
  public selection = new DatatableSelection();

  constructor(
    @Inject(ActivatedRoute) activatedRoute: ActivatedRoute,
    @Inject(AuthSessionService) authSessionService: AuthSessionService,
    @Inject(Router) router: Router,
    private blockUiService: BlockUiService,
    private clipboardService: ClipboardService,
    private dataStoreService: DataStoreService,
    private rpcService: RpcService,
    private dialogService: DialogService,
    private notificationService: NotificationService
  ) {
    super(activatedRoute, authSessionService, router);
  }

  /**
   * Append the current selection to the page context.
   */
  override get pageContext(): PageContext {
    const result = _.merge(
      {
        _selected: this.selection.selected
      },
      super.pageContext
    );
    return result;
  }

  loadData(params: DataTableLoadParams) {
    const store = this.config.store;
    if (_.isPlainObject(store.proxy)) {
      _.defaultsDeep(store.proxy.get, {
        params: {
          start: 0,
          limit: -1
        }
      });
      // Convert paging and sorting parameters.
      if (_.isNumber(params.offset) && _.isNumber(params.limit)) {
        _.merge(store.proxy.get.params, {
          start: params.offset * params.limit,
          limit: params.limit
        });
      }
      if (_.isString(params.dir) && _.isString(params.prop)) {
        _.merge(store.proxy.get.params, {
          sortdir: params.dir,
          sortfield: params.prop
        });
      }
      if (!_.isUndefined(params.search)) {
        _.merge(store.proxy.get.params, {
          search: params.search
        });
      }
    }
    this.loading = true;
    this.dataStoreService
      .load(store)
      .pipe(
        finalize(() => {
          this.loading = false;
        })
      )
      .subscribe(
        (res: RpcListResponse) => {
          // Update the total count of all rows.
          if (_.isPlainObject(store.proxy)) {
            this.count = res.total;
          }
        },
        () => {
          // Reset store and table in case of an error.
          store.data = [];
          this.count = 0;
        }
      );
  }

  /**
   * Reload the data to refresh/update the datatable content.
   */
  reloadData() {
    this.table.reloadData();
  }

  onSelectionChange(selection: DatatableSelection) {
    this.selection = selection;
  }

  onActionClick(action: DatatablePageActionConfig): void {
    const postConfirmFn = () => {
      switch (action?.execute?.type) {
        case 'url':
          const url: string = format(
            action.execute.url,
            _.merge(
              {},
              this.pageContext,
              this.selection.hasSingleSelection ? this.selection.first() : {}
            )
          );
          this.router.navigateByUrl(url);
          break;
        case 'request':
          const observables = [];
          const request = action.execute.request;
          if (this.selection.hasSelection) {
            this.selection.selected.forEach((selected) => {
              const params = formatDeep(request.params, _.merge({}, this.pageContext, selected));
              observables.push(
                this.rpcService[request.task ? 'requestTask' : 'request'](
                  request.service,
                  request.method,
                  params
                )
              );
            });
          } else {
            const params = formatDeep(request.params, this.pageContext);
            observables.push(
              this.rpcService[request.task ? 'requestTask' : 'request'](
                request.service,
                request.method,
                params
              )
            );
          }
          // Block UI and display the progress message.
          if (_.isString(request.progressMessage)) {
            this.blockUiService.start(translate(request.progressMessage));
          }
          // Process each request one-by-one (NOT in parallel).
          concat(...observables)
            .pipe(
              finalize(() => {
                if (_.isString(request.progressMessage)) {
                  this.blockUiService.stop();
                }
              })
            )
            .subscribe((res: any) => {
              const data: Record<any, any> = _.merge(
                {},
                this.pageContext,
                isFormatable(res) ? { _response: res } : {}
              );
              // Display a notification?
              if (_.isString(request.successNotification)) {
                const successNotification: string = format(request.successNotification, data);
                this.notificationService.show(
                  NotificationType.success,
                  undefined,
                  successNotification
                );
              }
              // Copy the response to the clipboard?
              if (_.isString(request.successCopyToClipboard)) {
                const successCopyToClipboard: string = format(request.successCopyToClipboard, data);
                this.clipboardService.copy(successCopyToClipboard);
              }
              // Navigate to the specified URL or reload the datatable
              // content.
              if (_.isString(request.successUrl)) {
                const successUrl: string = format(request.successUrl, data);
                this.router.navigateByUrl(successUrl);
              } else {
                this.reloadData();
              }
            });
          break;
        case 'taskDialog':
          const taskDialog = _.cloneDeep(action.execute.taskDialog);
          // Process tokenized configuration properties.
          _.forEach(['request.params'], (path) => {
            const value = _.get(taskDialog.config, path);
            if (isFormatable(value)) {
              _.set(taskDialog.config, path, formatDeep(value, this.pageContext));
            }
          });
          const dialog = this.dialogService.open(TaskDialogComponent, {
            width: _.get(taskDialog.config, 'width', '75%'),
            data: _.omit(taskDialog.config, ['width'])
          });
          dialog.afterClosed().subscribe((res) => {
            // Navigate to the configured URL or reload the datatable,
            // but only if the dialog close input is `true`.
            if (res) {
              if (_.isString(taskDialog.successUrl)) {
                this.navigate(taskDialog.successUrl);
              } else {
                this.reloadData();
              }
            }
          });
          break;
        case 'formDialog':
          const formDialogConfig = _.cloneDeep(action.execute.formDialog);
          // Process tokenized form field properties.
          _.forEach(formDialogConfig.fields, (fieldConfig: FormFieldConfig) => {
            _.forEach(['store.proxy', 'store.filters', 'value', 'request.params'], (path) => {
              const value = _.get(fieldConfig, path);
              if (isFormatable(value)) {
                _.set(fieldConfig, path, formatDeep(value, this.pageContext));
              }
            });
          });
          const formDialog = this.dialogService.open(FormDialogComponent, {
            width: _.get(formDialogConfig, 'width', '50%'),
            data: _.omit(formDialogConfig, ['width'])
          });
          // Reload datatable if pressed button returns `true`.
          formDialog.afterClosed().subscribe((res) => res && this.reloadData());
          break;
        case 'copyToClipboard':
          const copyToClipboard: string = format(
            action.execute.copyToClipboard,
            _.merge(
              {},
              this.pageContext,
              this.selection.hasSingleSelection ? this.selection.first() : {}
            )
          );
          this.clipboardService.copy(copyToClipboard);
          break;
      }
    };
    // Must the user confirm the action?
    if (_.isPlainObject(action.confirmationDialogConfig)) {
      const data = _.cloneDeep(action.confirmationDialogConfig);
      if (_.isString(data.message)) {
        data.message = format(data.message, this.pageContext);
      }
      const dialogRef = this.dialogService.open(ModalDialogComponent, {
        width: _.get(data, 'width'),
        data: _.omit(data, ['width'])
      });
      dialogRef.afterClosed().subscribe((res) => {
        if (true === res) {
          postConfirmFn();
        }
      });
    } else {
      postConfirmFn();
    }
  }

  onDeleteActionClick(action: DatatableAction) {
    let message: string = gettext('Do you really want to delete the selected item(s)?');
    if (isFormatable(this.config.rowEnumFmt)) {
      const items: Array<string> = _.map(this.selection.selected, (selected) =>
        format(this.config.rowEnumFmt, selected)
      );
      message = format(
        gettext(
          'Do you really want to delete the selected item(s) <strong>{{ items | join(", ") }}</strong>?'
        ),
        { items }
      );
    }
    this.onActionClick(
      _.merge(
        {
          confirmationDialogConfig: {
            template: 'confirmation-danger',
            title: gettext('Delete'),
            message
          },
          execute: {
            request: {
              progressMessage: gettext('Please wait, deleting selected item(s) ...')
            }
          }
        } as any,
        _.omit(action, 'click')
      )
    );
  }

  onButtonClick(buttonConfig: DatatablePageButtonConfig) {
    if (_.isFunction(buttonConfig.click)) {
      buttonConfig.click(buttonConfig, this.config.store);
    } else {
      this.navigate(buttonConfig.url);
    }
  }

  protected override sanitizeConfig() {
    _.defaultsDeep(this.config, {
      columnMode: 'flex',
      hasActionBar: true,
      hasHeader: true,
      hasFooter: true,
      selectionType: 'multi',
      updateSelectionOnReload: 'always',
      rowId: 'uuid',
      limit: 25,
      remotePaging: false,
      remoteSorting: false,
      remoteSearching: false,
      autoLoad: true,
      autoReload: false,
      columns: [],
      actions: [],
      sorters: [],
      sortType: 'single',
      buttonAlign: 'end',
      buttons: []
    });
    // Map icon from 'foo' to 'mdi:foo' if necessary.
    this.config.icon = _.get(Icon, this.config.icon, this.config.icon);
    // Pre-setup actions based on the specified template type.
    this.sanitizeActionsConfig(this.config.actions);
    // Set the default hint properties.
    this.sanitizeHintsConfig();
    // Set the default values of the buttons.
    _.forEach(this.config.buttons, (button) => {
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
    // Relocate the 'submit' button to the end of the list.
    const index = _.findIndex(this.config.buttons, ['template', 'submit']);
    if (index !== -1) {
      const button = this.config.buttons[index];
      this.config.buttons.splice(index, 1);
      this.config.buttons.push(button);
    }
  }

  protected override onRouteParams() {
    // Format tokenized configuration properties.
    this.formatConfig([
      'title',
      'subTitle',
      'store.proxy.service',
      'store.proxy.get.method',
      'store.proxy.get.params',
      'store.filters'
    ]);
    this.formatHintsConfig();
  }

  private sanitizeActionsConfig(actions: DatatablePageActionConfig[]) {
    _.forEach(actions, (action: DatatablePageActionConfig) => {
      _.defaultsDeep(action, {
        click: this.onActionClick.bind(this)
      });
      if (_.isArray(action.actions)) {
        this.sanitizeActionsConfig(action.actions);
      }
      // Map icon from 'foo' to 'mdi:foo' if necessary.
      action.icon = _.get(Icon, action.icon, action.icon);
      // Process templates.
      switch (action.template) {
        case 'add':
          _.defaultsDeep(action, {
            id: 'add',
            type: 'iconButton',
            text: gettext('Add'),
            tooltip: gettext('Add'),
            icon: Icon.add
          });
          break;
        case 'create':
          _.defaultsDeep(action, {
            id: 'create',
            type: 'iconButton',
            text: gettext('Create'),
            tooltip: gettext('Create'),
            icon: Icon.add
          });
          break;
        case 'edit':
          _.defaultsDeep(action, {
            id: 'edit',
            type: 'iconButton',
            text: gettext('Edit'),
            tooltip: gettext('Edit'),
            icon: Icon.edit,
            enabledConstraints: {
              minSelected: 1,
              maxSelected: 1
            }
          });
          break;
        case 'delete':
          _.defaultsDeep(action, {
            id: 'delete',
            type: 'iconButton',
            text: gettext('Delete'),
            tooltip: gettext('Delete'),
            icon: Icon.delete,
            enabledConstraints: {
              minSelected: 1
            }
          });
          _.merge(action, {
            click: this.onDeleteActionClick.bind(this)
          });
          break;
        default:
          _.defaultsDeep(action, {
            execute: {
              request: {
                progressMessage: gettext('Please wait, processing selected item(s) ...')
              }
            }
          });
          break;
      }
    });
  }

  private navigate(url: string) {
    const formattedUrl = format(url, this.pageContext);
    this.router.navigateByUrl(formattedUrl);
  }
}
