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
/* eslint-disable @typescript-eslint/member-ordering */
import { BooleanInput, coerceBooleanProperty } from '@angular/cdk/coercion';
import { Component, Input, OnChanges, OnDestroy, OnInit, Optional, Self } from '@angular/core';
import { ControlValueAccessor, NgControl } from '@angular/forms';
import { MatFormFieldControl } from '@angular/material/form-field';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { Subject } from 'rxjs';

import { FormDialogComponent } from '~/app/core/components/intuition/form-dialog/form-dialog.component';
import { FormFieldConfig } from '~/app/core/components/intuition/models/form-field-config.type';
import { CoerceBoolean } from '~/app/decorators';
import { formatDeep } from '~/app/functions.helper';
import { DataTableCellChanged } from '~/app/shared/components/datatable/datatable.component';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { DataStore } from '~/app/shared/models/data-store.type';
import { Datatable } from '~/app/shared/models/datatable.interface';
import { DatatableAction } from '~/app/shared/models/datatable-action.type';
import { DatatableColumn } from '~/app/shared/models/datatable-column.type';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';
import { Sorter } from '~/app/shared/models/sorter.type';
import { DataStoreService } from '~/app/shared/services/data-store.service';
import { DialogService } from '~/app/shared/services/dialog.service';

let nextUniqueId = 0;

export type MatFormDatatableAction = {
  // Specifies a template which pre-configures the action button.
  // add -    Shows a form dialog. When the dialog is successfully
  //          closed, then the form values will be used to add a new
  //          row to the datatable.
  // edit -   Shows a form dialog which displays the data of the
  //          current selected row. The action button is only enabled
  //          when one row is selected. When the dialog is
  //          successfully closed, then the form values are used
  //          to update the current selected row.
  // delete - The action button is only enabled when one row is
  //          selected. If pressed, the current selected row will
  //          be removed from the datatable.
  template?: 'add' | 'edit' | 'delete';
  formDialogConfig?: {
    // The dialog title.
    title?: string;
    // Width of the dialog in 'px' or '%'. Defaults to '50%'.
    width?: string;
    // The form fields that is displayed in the dialog when the
    // 'Add' or 'Edit' button is pressed.
    fields?: Array<FormFieldConfig>;
  };
  // Transform the given keys in the form values after the dialog
  // has been closed and before the datatable row will be added or
  // updated.
  // Example:
  // Values = { foo: 'bar', num: '3', str: 'xyzzzz' }
  // transform = {
  //   foo: 'baz-{{ foo }}',
  //   num: '{{ num | int }}',
  //   str: '{{ str | strip("z") }}',
  //   add: 'aaa'
  // }
  // Result = { foo: 'baz-bar', num: 3, str: 'xy', add: 'aaa' }
  transform?: { [key: string]: string };
  // Internal
  icon?: string;
  tooltip?: string;
  click?: () => void;
};

@Component({
  // eslint-disable-next-line @angular-eslint/component-selector
  selector: 'mat-form-datatable',
  exportAs: 'matFormDatatable',
  templateUrl: './mat-form-datatable.component.html',
  styleUrls: ['./mat-form-datatable.component.scss'],
  providers: [
    {
      provide: MatFormFieldControl,
      useExisting: MatFormDatatableComponent
    }
  ]
})
export class MatFormDatatableComponent
  implements ControlValueAccessor, MatFormFieldControl<any[]>, OnInit, OnDestroy, OnChanges
{
  // eslint-disable-next-line @typescript-eslint/naming-convention
  static ngAcceptInputType_disabled: BooleanInput;
  // eslint-disable-next-line @typescript-eslint/naming-convention
  static ngAcceptInputType_required: BooleanInput;

  @Input()
  get value() {
    return this._value;
  }
  set value(value) {
    if (!_.isEqual(value, this._value)) {
      this.store.data = value;
      this.dataStoreService.load(this.store).subscribe(() => {
        this._value = _.cloneDeep(value);
        this.onChange(this.value);
        this.stateChanges.next();
      });
    }
  }

  @Input()
  get required(): boolean {
    return this._required;
  }
  set required(value: boolean) {
    this._required = coerceBooleanProperty(value);
    this.stateChanges.next();
  }

  @Input()
  get disabled(): boolean {
    return this._disabled;
  }
  set disabled(value: boolean) {
    this._disabled = coerceBooleanProperty(value);
    this.stateChanges.next();
  }

  @Input()
  get id(): string {
    return this._uniqueId;
  }

  constructor(
    @Optional() @Self() public ngControl: NgControl,
    private dataStoreService: DataStoreService,
    private dialogService: DialogService
  ) {
    // Replace the provider from above with this.
    if (!_.isNull(this.ngControl)) {
      // Setting the value accessor directly (instead of using
      // the providers) to avoid running into a circular import.
      this.ngControl.valueAccessor = this;
    }
  }

  /***********************************************************************
   * Material related methods
   **********************************************************************/

  get empty(): boolean {
    return !this._value.length;
  }

  @Input()
  columns: DatatableColumn[] = [];

  @Input()
  columnMode?: 'standard' | 'flex' | 'force' = 'flex';

  @Input()
  actions: MatFormDatatableAction[] = [];

  @CoerceBoolean()
  @Input()
  hasActionBar? = true;

  @CoerceBoolean()
  @Input()
  hasSearchField? = false;

  @CoerceBoolean()
  @Input()
  hasHeader? = true;

  @CoerceBoolean()
  @Input()
  hasFooter? = true;

  @Input()
  limit? = 25;

  @Input()
  sorters?: Sorter[] = [];

  @Input()
  sortType?: 'single' | 'multi' = 'single';

  @Input()
  selectionType?: 'none' | 'single' | 'multi' = 'multi';

  @Input()
  valueType?: 'string' | 'integer' | 'number' | 'object' = 'object';

  public controlType = 'datatable';
  public stateChanges = new Subject<void>();
  public shouldLabelFloat = true;
  public errorState = false;
  public focused = false;
  public placeholder = '';
  public selection = new DatatableSelection();
  public store: DataStore = { data: [] };

  // eslint-disable-next-line @typescript-eslint/naming-convention,no-underscore-dangle,id-blacklist,id-match
  private _value = [];
  // eslint-disable-next-line @typescript-eslint/naming-convention,no-underscore-dangle,id-blacklist,id-match
  private _required = false;
  // eslint-disable-next-line @typescript-eslint/naming-convention,no-underscore-dangle,id-blacklist,id-match
  private _disabled = false;
  // eslint-disable-next-line @typescript-eslint/naming-convention,no-underscore-dangle,id-blacklist,id-match
  private _uniqueId = `mat-datatable-${++nextUniqueId}`;
  // eslint-disable-next-line @typescript-eslint/naming-convention, no-underscore-dangle, id-blacklist, id-match
  private onChange = (_value: any) => {};

  writeValue(value: any): void {
    this.value = value;
  }

  setDisabledState(isDisabled: boolean): void {
    this.disabled = isDisabled;
  }

  registerOnChange(fn: (value: any) => void): void {
    this.onChange = fn;
  }

  registerOnTouched(fn: any): void {}

  setDescribedByIds(ids: string[]): void {
    // Nothing to do here.
  }

  onContainerClick(event: MouseEvent): void {
    // Nothing to do here.
  }

  /***********************************************************************
   * Component related methods
   **********************************************************************/

  ngOnInit(): void {
    this.sanitizeConfig();
    // Populate the store fields based on the configured columns.
    this.store.fields = _.uniq(_.flatMap(this.columns, (column) => column.prop as string));
  }

  ngOnDestroy(): void {
    this.stateChanges.complete();
  }

  ngOnChanges(): void {
    this.stateChanges.next();
  }

  private sanitizeConfig() {
    this.actions.forEach((action: DatatableAction) => {
      _.defaultsDeep(action, {
        click: this.onActionClick.bind(this)
      });
    });
  }

  onSelectionChange(selection: DatatableSelection) {
    this.selection = selection;
  }

  onCellDataChanged(data: DataTableCellChanged) {
    // The data store (`this.store.data`) contains the latest values.
    // The internal representation needs to be updated.
    this.syncValue();
  }

  onActionClick(action: DatatableAction, table: Datatable) {
    const actionConfig = _.find(this.actions, { id: action.id });
    switch (action.id) {
      case 'add':
      case 'edit':
        const formDialogConfig: Record<string, any> = _.get(actionConfig, 'formDialogConfig');
        const data = {
          title: _.get(formDialogConfig, 'title'),
          fields: _.get(formDialogConfig, 'fields')
        };
        if ('add' === action.id) {
          _.defaultsDeep(data, {
            buttons: {
              submit: {
                text: gettext('Add')
              }
            }
          });
        }
        const formDialogRef = this.dialogService.open(FormDialogComponent, {
          data,
          width: _.get(formDialogConfig, 'width', '50%')
        });
        if ('edit' === action.id) {
          // Update the form field values.
          formDialogRef.afterOpened().subscribe(() => {
            formDialogRef.componentInstance.setFormValues(table.selection.first());
          });
        }
        formDialogRef.afterClosed().subscribe((res) => {
          if (res) {
            // Apply value transformation.
            if (_.isPlainObject(formDialogConfig.transform)) {
              const tmp = formatDeep(formDialogConfig.transform, res);
              _.merge(res, tmp);
            }
            switch (action.id) {
              case 'add':
                this.store.data.push(res);
                break;
              case 'edit':
                // We can process the `table.selection.selected` here
                // because the objects are referencing the objects in
                // `this.store.data`.
                const selected = table.selection.first();
                _.assign(selected, res);
                break;
            }
            this.syncValue();
          }
        });
        break;
      case 'delete':
        const modalDialogRef = this.dialogService.open(ModalDialogComponent, {
          data: {
            template: 'confirmation',
            title: gettext('Delete'),
            message: gettext('Do you really want to delete the selected item(s)?'),
            buttons: [{}, { class: 'omv-background-color-pair-red' }]
          }
        });
        modalDialogRef.afterClosed().subscribe((res) => {
          if (res) {
            this.store.data = _.pullAllWith(this.store.data, table.selection.selected, _.isEqual);
            this.syncValue();
          }
        });
        break;
    }
  }

  private syncValue() {
    const value = [];
    _.forEach(this.store.data, (item: any) => {
      switch (this.valueType) {
        case 'integer':
        case 'number':
        case 'string':
          // Use the first column of all columns to get the property
          // name to be processed.
          const column: DatatableColumn = _.first(this.columns);
          item = _.get(item, column.prop);
          // Convert the value.
          switch (this.valueType) {
            case 'integer':
              item = _.toInteger(item);
              break;
            case 'number':
              item = _.toNumber(item);
              break;
            case 'string':
              item = _.toString(item);
              break;
          }
          break;
        case 'object':
          // Nothing to do here.
          break;
      }
      value.push(item);
    });
    this.writeValue(value);
  }
}
