/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { concat, Observable, Subscription } from 'rxjs';
import { finalize } from 'rxjs/operators';

import { AbstractPageComponent } from '~/app/core/components/intuition/abstract-page-component';
import { FormDialogComponent } from '~/app/core/components/intuition/form-dialog/form-dialog.component';
import { DatatablePageActionConfig } from '~/app/core/components/intuition/models/datatable-page-action-config.type';
import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { DatatablePageButtonConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { FormFieldConfig } from '~/app/core/components/intuition/models/form-field-config.type';
import { PageContextService } from '~/app/core/services/page-context.service';
import { Unsubscribe } from '~/app/decorators';
import { format, formatDeep, isFormatable } from '~/app/functions.helper';
import { translate } from '~/app/i18n.helper';
import {
  DatatableComponent,
  DataTableLoadParams
} from '~/app/shared/components/datatable/datatable.component';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { TaskDialogComponent } from '~/app/shared/components/task-dialog/task-dialog.component';
import { DEFAULT_TEXTS } from '~/app/shared/constants/text.constants';
import { Icon } from '~/app/shared/enum/icon.enum';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { DataStore } from '~/app/shared/models/data-store.type';
import { DatatableAction } from '~/app/shared/models/datatable-action.type';
import { DatatableData } from '~/app/shared/models/datatable-data.type';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { DataStoreResponse, DataStoreService } from '~/app/shared/services/data-store.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { NotificationService } from '~/app/shared/services/notification.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-intuition-datatable-page',
  templateUrl: './datatable-page.component.html',
  styleUrls: ['./datatable-page.component.scss'],
  providers: [PageContextService]
})
export class DatatablePageComponent extends AbstractPageComponent<DatatablePageConfig> {
  @ViewChild('table', { static: true })
  table: DatatableComponent;

  @Unsubscribe()
  private subscriptions = new Subscription();

  protected count = 0;
  protected selection = new DatatableSelection();

  // The unformatted RPC parameters of the store proxy. They are used
  // to load the child rows in tree mode.
  private treeParams: Record<string, any>;
  // The IDs of the expanded rows in tree mode. These rows are expanded
  // again after the data has been reloaded.
  private expandedTreeRowIds = new Set<any>();

  constructor(
    @Inject(PageContextService) pageContextService: PageContextService,
    private blockUiService: BlockUiService,
    private clipboardService: ClipboardService,
    private dataStoreService: DataStoreService,
    private router: Router,
    private rpcService: RpcService,
    private dialogService: DialogService,
    private notificationService: NotificationService
  ) {
    super(pageContextService);
    this.pageContextService.set({
      _selected: this.selection.selected
    });
  }

  onLoadDataEvent(params: DataTableLoadParams) {
    this.subscriptions.add(
      this.loadData(params).subscribe({
        next: (res: DataStoreResponse) => {
          // Update the total count of all rows.
          if (_.isPlainObject(this.config.store.proxy)) {
            this.count = res.total;
          }
          this.expandTreeRows(res.data);
        },
        error: () => {
          // Reset store and table in case of an error.
          this.config.store.data = [];
          this.count = 0;
        }
      })
    );
  }

  /**
   * Reload the data to refresh/update the datatable content.
   */
  reloadData() {
    this.table.reloadData();
  }

  /**
   * Expand or collapse the given row in tree mode. The child rows are
   * loaded when the row is expanded for the first time.
   */
  onTreeAction(row: DatatableData): void {
    const id = _.get(row, this.config.treeToRelation);
    switch (row.treeStatus) {
      case 'collapsed':
        this.expandedTreeRowIds.add(id);
        if (_.some(this.config.store.data, [this.config.treeFromRelation, id])) {
          row.treeStatus = 'expanded';
        } else {
          row.treeStatus = 'loading';
          this.loadTreeChildRows(row);
        }
        break;
      case 'expanded':
        this.expandedTreeRowIds.delete(id);
        row.treeStatus = 'collapsed';
        break;
    }
    // Force the datatable to rebuild the tree.
    this.config.store.data = [...this.config.store.data];
  }

  onSelectionChange(selection: DatatableSelection) {
    this.selection = selection;
    this.pageContextService.set({
      _selected: this.selection.selected
    });
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
            height: _.get(taskDialog.config, 'height', '75%'),
            data: _.omit(taskDialog.config, ['width', 'height'])
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
            title: DEFAULT_TEXTS.delete,
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
            text: DEFAULT_TEXTS.back
          });
          break;
        case 'cancel':
          _.defaultsDeep(button, {
            text: DEFAULT_TEXTS.cancel
          });
          break;
        case 'submit':
          _.defaultsDeep(button, {
            text: DEFAULT_TEXTS.save
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

  protected override onPageInit() {
    if (this.config.treeFromRelation && this.config.treeToRelation) {
      this.treeParams = _.cloneDeep(_.get(this.config.store, 'proxy.get.params', {}));
    }
    // Format tokenized configuration properties.
    this.formatConfig([
      'store.proxy.service',
      'store.proxy.get.method',
      'store.proxy.get.params',
      'store.proxy.post.method',
      'store.proxy.post.params',
      'store.filters'
    ]);
  }

  protected override doLoadData(params: DataTableLoadParams): Observable<DataStoreResponse> {
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
    return this.dataStoreService.load(store);
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
            text: DEFAULT_TEXTS.add,
            tooltip: DEFAULT_TEXTS.add,
            icon: Icon.add
          });
          break;
        case 'create':
          _.defaultsDeep(action, {
            id: 'create',
            type: 'iconButton',
            text: DEFAULT_TEXTS.create,
            tooltip: DEFAULT_TEXTS.create,
            icon: Icon.add
          });
          break;
        case 'edit':
          _.defaultsDeep(action, {
            id: 'edit',
            type: 'iconButton',
            text: DEFAULT_TEXTS.edit,
            tooltip: DEFAULT_TEXTS.edit,
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
            text: DEFAULT_TEXTS.delete,
            tooltip: DEFAULT_TEXTS.delete,
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

  private loadTreeChildRows(row: DatatableData): void {
    if (!_.isPlainObject(this.config.store.proxy)) {
      row.treeStatus = 'disabled';
      return;
    }
    const store: DataStore = _.cloneDeep(_.omit(this.config.store, 'data'));
    store.proxy.get.params = _.defaults(
      formatDeep(this.treeParams, _.merge({}, this.pageContext, { _parent: row })),
      { start: 0, limit: -1 }
    );
    this.subscriptions.add(
      this.dataStoreService.load(store).subscribe({
        next: (res: DataStoreResponse) => {
          // Discard the response if the data has been reloaded meanwhile.
          if (!_.includes(this.config.store.data, row)) {
            return;
          }
          // Ignore rows that already exist, e.g. if the store proxy
          // always returns the whole tree.
          const ids = new Set(_.map(this.config.store.data, this.config.treeToRelation));
          const childRows = _.reject(res.data, (childRow: DatatableData) =>
            ids.has(_.get(childRow, this.config.treeToRelation))
          );
          _.forEach(childRows, (childRow: DatatableData) => {
            if (!_.has(childRow, this.config.treeFromRelation)) {
              _.set(childRow, this.config.treeFromRelation, _.get(row, this.config.treeToRelation));
            }
          });
          row.treeStatus = childRows.length > 0 ? 'expanded' : 'disabled';
          this.config.store.data = [...this.config.store.data, ...childRows];
          this.expandTreeRows(childRows);
        },
        error: () => {
          row.treeStatus = 'collapsed';
          this.config.store.data = [...this.config.store.data];
        }
      })
    );
  }

  /**
   * Expand the given rows if they were expanded before the data has
   * been reloaded.
   */
  private expandTreeRows(rows: DatatableData[]): void {
    if (!this.config.treeFromRelation || !this.config.treeToRelation) {
      return;
    }
    _.forEach(rows, (row: DatatableData) => {
      if (
        row.treeStatus === 'collapsed' &&
        this.expandedTreeRowIds.has(_.get(row, this.config.treeToRelation))
      ) {
        this.onTreeAction(row);
      }
    });
  }
}
