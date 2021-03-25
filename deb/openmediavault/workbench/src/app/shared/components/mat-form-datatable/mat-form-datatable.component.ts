/* eslint-disable @typescript-eslint/member-ordering */
import { BooleanInput, coerceBooleanProperty } from '@angular/cdk/coercion';
import { Component, Input, OnChanges, OnDestroy, OnInit, Optional, Self } from '@angular/core';
import { ControlValueAccessor, NgControl } from '@angular/forms';
import { MatDialog } from '@angular/material/dialog';
import { MatFormFieldControl } from '@angular/material/form-field';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { Subject } from 'rxjs';

import { FormDialogComponent } from '~/app/core/components/limn-ui/form-dialog/form-dialog.component';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { DataStore } from '~/app/shared/models/data-store.type';
import { DatatableActionConfig } from '~/app/shared/models/datatable-action-config.type';
import { DatatableColumn } from '~/app/shared/models/datatable-column.type';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';
import { Sorter } from '~/app/shared/models/sorter.type';
import { DataStoreService } from '~/app/shared/services/data-store.service';

let nextUniqueId = 0;

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
  implements ControlValueAccessor, MatFormFieldControl<any[]>, OnInit, OnDestroy, OnChanges {
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
        this._value = value;
        this.onChange(value);
        this.stateChanges.next();
      });
    }
  }

  @Input()
  get required(): boolean {
    return this._required;
  }
  set required(required: boolean) {
    this._required = coerceBooleanProperty(required);
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
    private matDialog: MatDialog
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
  actions: DatatableActionConfig[] = [];

  @Input()
  hasActionBar? = true;

  @Input()
  hasSearchField? = false;

  @Input()
  hasHeader? = true;

  @Input()
  hasFooter? = true;

  @Input()
  limit? = 25;

  @Input()
  sorters?: Sorter[] = [];

  @Input()
  selectionType?: 'single' | 'multi' = 'multi';

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

  writeValue(value: any) {
    this.value = value;
  }

  setDisabledState(isDisabled: boolean): void {
    this.disabled = isDisabled;
  }

  registerOnChange(fn: (value: any) => void) {
    this.onChange = fn;
  }

  registerOnTouched(fn: any) {}

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
    this.actions.forEach((action) => {
      _.defaultsDeep(action, {
        click: this.onActionClick.bind(this)
      });
    });
  }

  onSelectionChange(selection: DatatableSelection) {
    this.selection = selection;
  }

  onActionClick(action: DatatableActionConfig, selection: DatatableSelection) {
    const actionConfig = _.find(this.actions, { id: action.id });
    switch (action.id) {
      case 'add':
      case 'edit':
        const dialogConfig = _.get(actionConfig, 'dialogConfig');
        const data = {
          title: _.get(dialogConfig, 'title'),
          fields: _.get(dialogConfig, 'fields')
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
        const formDialogRef = this.matDialog.open(FormDialogComponent, {
          data,
          width: _.get(dialogConfig, 'width', '50%'),
          height: _.get(dialogConfig, 'height')
        });
        if ('edit' === action.id) {
          // Update form field values.
          formDialogRef.afterOpened().subscribe(() => {
            formDialogRef.componentInstance.setFormValues(selection.first());
          });
        }
        formDialogRef.afterClosed().subscribe((res) => {
          if (res) {
            this.store.data.push(res);
            this.syncValue();
          }
        });
        break;
      case 'delete':
        const modalDialogRef = this.matDialog.open(ModalDialogComponent, {
          data: {
            template: 'confirmation',
            title: gettext('Delete'),
            message: gettext('Do you really want to delete the selected item(s)?'),
            buttons: [{}, { class: 'omv-background-color-theme-red' }]
          }
        });
        modalDialogRef.afterClosed().subscribe((res) => {
          if (res) {
            this.store.data = _.pullAllWith(this.store.data, selection.selected, _.isEqual);
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
