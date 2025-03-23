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
import {
  Component,
  EventEmitter,
  Input,
  OnChanges,
  OnDestroy,
  OnInit,
  Output,
  SimpleChange,
  SimpleChanges,
  TemplateRef,
  ViewChild,
  ViewEncapsulation
} from '@angular/core';
import { MatButtonToggleChange } from '@angular/material/button-toggle';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import { DatatableComponent as NgxDatatableComponent } from '@siemens/ngx-datatable';
import * as _ from 'lodash';
import { Subscription, timer } from 'rxjs';

import { CoerceBoolean, Throttle, Unsubscribe } from '~/app/decorators';
import { translate } from '~/app/i18n.helper';
import { Icon } from '~/app/shared/enum/icon.enum';
import { Datatable } from '~/app/shared/models/datatable.interface';
import { DatatableColumn } from '~/app/shared/models/datatable-column.type';
import { DatatableData } from '~/app/shared/models/datatable-data.type';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';
import { Sorter } from '~/app/shared/models/sorter.type';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

export type DataTableLoadParams = {
  dir?: 'asc' | 'desc';
  prop?: string;
  offset?: number;
  limit?: number;
  search?: any;
};

export type DataTableCellChanged = {
  value: any;
  prop: string | number;
  row: Record<string, DatatableData | null>;
  column: DatatableColumn;
};

@Component({
  selector: 'omv-datatable',
  templateUrl: './datatable.component.html',
  styleUrls: ['./datatable.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class DatatableComponent implements Datatable, OnInit, OnDestroy, OnChanges {
  @ViewChild('table', { static: true })
  table: NgxDatatableComponent;
  @ViewChild('textTpl', { static: true })
  textTpl: TemplateRef<any>;
  @ViewChild('htmlTpl', { static: true })
  htmlTpl: TemplateRef<any>;
  @ViewChild('imageTpl', { static: true })
  imageTpl: TemplateRef<any>;
  @ViewChild('checkIconTpl', { static: true })
  checkIconTpl: TemplateRef<any>;
  @ViewChild('checkBoxTpl', { static: true })
  checkBoxTpl: TemplateRef<any>;
  @ViewChild('joinTpl', { static: true })
  joinTpl: TemplateRef<any>;
  @ViewChild('truncateTpl', { static: true })
  truncateTpl: TemplateRef<any>;
  @ViewChild('placeholderTpl', { static: true })
  placeholderTpl: TemplateRef<any>;
  @ViewChild('progressBarTpl', { static: true })
  progressBarTpl: TemplateRef<any>;
  @ViewChild('notAvailableTpl', { static: true })
  notAvailableTpl: TemplateRef<any>;
  @ViewChild('shapeShifterTpl', { static: true })
  shapeShifterTpl: TemplateRef<any>;
  @ViewChild('localeDateTimeTpl', { static: true })
  localeDateTimeTpl: TemplateRef<any>;
  @ViewChild('relativeTimeTpl', { static: true })
  relativeTimeTpl: TemplateRef<any>;
  @ViewChild('chipTpl', { static: true })
  chipTpl: TemplateRef<any>;
  @ViewChild('binaryUnitTpl', { static: true })
  binaryUnitTpl: TemplateRef<any>;
  @ViewChild('unsortedListTpl', { static: true })
  unsortedListTpl: TemplateRef<any>;
  @ViewChild('templateTpl', { static: true })
  templateTpl: TemplateRef<any>;
  @ViewChild('buttonToggleTpl', { static: true })
  buttonToggleTpl: TemplateRef<any>;
  @ViewChild('copyToClipboardTpl', { static: true })
  copyToClipboardTpl: TemplateRef<any>;
  @ViewChild('cronToHumanTpl', { static: true })
  cronToHumanTpl: TemplateRef<any>;

  // Define a query selector if the datatable is used in an
  // overflow container.
  @Input()
  ownerContainer?: string;

  // The data to be shown.
  @Input()
  data: DatatableData[];

  // Show the linear loading bar.
  @CoerceBoolean()
  @Input()
  loadingIndicator? = false;

  // An identifier, e.g. a UUID, which identifies this datatable
  // uniquely. This is used to store/restore the column state.
  @Input()
  stateId?: string;

  // The name of the property that identifies a row uniquely.
  @Input()
  rowId?: string;

  @Input()
  columnMode?: 'standard' | 'flex' | 'force' = 'flex';

  @CoerceBoolean()
  @Input()
  reorderable? = false;

  // Display the toolbar above the datatable that includes
  // the custom and default (e.g. 'Reload') action buttons?
  @CoerceBoolean()
  @Input()
  hasActionBar? = true;

  // Use a fixed action bar so that it does not leave the viewport
  // even when scrolled.
  @CoerceBoolean()
  @Input()
  hasStickyActionBar? = false;

  // Show/Hide the reload button. If 'autoReload' is set to `true`,
  // then the button is automatically hidden.
  @CoerceBoolean()
  @Input()
  hasReloadButton? = true;

  // Show/Hide the search field. Defaults to `false`.
  @CoerceBoolean()
  @Input()
  hasSearchField? = false;

  // Display the datatable header?
  @CoerceBoolean()
  @Input()
  hasHeader? = true;

  // Display the datatable footer?
  @CoerceBoolean()
  @Input()
  hasFooter? = true;

  @Input()
  selectionType?: 'none' | 'single' | 'multi' = 'multi';

  // By default, selected items will be updated on reload.
  @Input()
  updateSelectionOnReload: 'always' | 'onChange' | 'never' = 'always';

  // Automatically load the data after datatable has been
  // initialized. If set to false, the autoReload configuration
  // is not taken into action. Defaults to `true`.
  @CoerceBoolean()
  @Input()
  autoLoad? = true;

  // The frequency in milliseconds with which the data
  // should be reloaded. Defaults to `false`.
  @Input()
  autoReload?: boolean | number = false;

  // Page size to show. To disable paging, set the limit to 0.
  // Defaults to 25.
  @Input()
  limit? = 25;

  // Total count of all rows.
  @Input()
  count? = 0;

  // Use remote paging instead of client-side.
  @CoerceBoolean()
  @Input()
  remotePaging = false;

  // Use remote sorting instead of client-side.
  @CoerceBoolean()
  @Input()
  remoteSorting = false;

  // Use remote searching instead of client-side.
  @CoerceBoolean()
  @Input()
  remoteSearching = false;

  // Sorting mode. In "single" mode, clicking on a column name will
  // reset the existing sorting before sorting by the new selection.
  // In multi selection mode, additional clicks on column names will
  // add sorting using multiple columns.
  @Input()
  sortType?: 'single' | 'multi' = 'single';

  // Ordered array of objects used to determine sorting by column.
  @Input()
  sorters?: Sorter[] = [];

  // Event emitted when the data must be loaded.
  @Output()
  readonly loadDataEvent = new EventEmitter<DataTableLoadParams>();

  // Event emitted when the selection has been changed.
  // Note, the `DatatableSelection` event object is a deep copy of
  // the internal object, so manipulations of the object will not
  // affect the data displayed in the table.
  @Output()
  readonly selectionChangeEvent = new EventEmitter<DatatableSelection>();

  // Event emitted when the cell data has been changed.
  // This applies only to columns of type 'buttonToggle'.
  @Output()
  readonly cellDataChangedEvent = new EventEmitter<DataTableCellChanged>();

  @Unsubscribe()
  private subscriptions = new Subscription();

  // Internal
  public icon = Icon;
  public rows = [];
  public offset = 0;
  public selection = new DatatableSelection();
  public filteredColumns: DatatableColumn[];
  public messages: {
    emptyMessage: string;
    totalMessage: string;
    selectedMessage: string;
  };
  public searchFilter = '';

  private cellTemplates: { [key: string]: TemplateRef<any> };
  private rawColumns: DatatableColumn[] = [];

  constructor(
    private clipboardService: ClipboardService,
    private userLocalStorageService: UserLocalStorageService
  ) {
    this.messages = {
      emptyMessage: translate(gettext('No data to display.')),
      totalMessage: translate(gettext('total')),
      selectedMessage: translate(gettext('selected'))
    };
  }

  // The column configuration.
  @Input()
  get columns(): DatatableColumn[] {
    return this.rawColumns;
  }
  set columns(columns: DatatableColumn[]) {
    this.sanitizeColumns(columns);
    this.rawColumns = [...columns];
  }

  @Throttle(1000)
  onSearchFilterChange(): void {
    if (!this.remoteSearching) {
      this.applySearchFilter();
    } else {
      this.reloadData();
    }
  }

  ngOnInit(): void {
    // Init cell templates.
    this.initTemplates();
    // Sanitize configuration.
    this.sanitizeConfig();
    // Initialize timer or simply load the data once.
    // Note, we'll also use the RxJS timer when loading the data only once,
    // that's because this will prevent us from getting an 'Expression has
    // changed after it was checked' error.
    if (this.autoLoad) {
      const period = _.isNumber(this.autoReload) ? (this.autoReload as number) : null;
      this.subscriptions.add(
        timer(0, period).subscribe(() => {
          this.reloadData();
        })
      );
    }
    this.updateColumns();
  }

  ngOnDestroy(): void {
    (this.onSearchFilterChange as any).cancel?.();
  }

  ngOnChanges(changes: SimpleChanges): void {
    _.forIn(changes, (_change: SimpleChange, propName: string) => {
      switch (propName) {
        case 'data': {
          if (!this.data) {
            return;
          }
          if (!this.remoteSearching && this.searchFilter !== '') {
            this.applySearchFilter();
          } else {
            this.rows = [...this.data];
          }
          this.updateSelection();
        }
      }
    });
  }

  /**
   * Reload the data to be shown by emitting the 'loadDataEvent' event.
   */
  reloadData(): void {
    const params: DataTableLoadParams = {};
    if (this.remotePaging) {
      _.merge(params, { offset: this.offset, limit: this.limit });
    }
    if (this.remoteSorting && !_.isEmpty(this.sorters)) {
      _.merge(params, { dir: this.sorters[0].dir, prop: this.sorters[0].prop });
    }
    if (this.remoteSearching) {
      _.merge(params, { search: this.searchFilter });
    }
    this.loadDataEvent.emit(params);
  }

  /**
   * Update the data to be shown.
   * The internal data structures are updated and the table will
   * be redrawn.
   */
  updateData(data: DatatableData[]): void {
    this.data.splice(0, this.data.length, ...data);
    this.rows = [...this.data];
  }

  updateSelection(): void {
    if (this.updateSelectionOnReload === 'never') {
      return;
    }
    // Get the new selected rows.
    const newSelected: any[] = [];
    _.forEach(this.selection.selected, (item) => {
      const row = _.find(this.data, [this.rowId, _.get(item, this.rowId)]);
      if (!_.isUndefined(row)) {
        newSelected.push(row);
      }
    });
    if (
      this.updateSelectionOnReload === 'onChange' &&
      _.isEqual(this.selection.selected, newSelected)
    ) {
      return;
    }
    this.selection.set(newSelected);
    this.onSelect();
  }

  onSelect(): void {
    // Make a deep copy of the selection and emit it to the subscribers.
    this.selectionChangeEvent.emit(_.cloneDeep(this.selection));
  }

  onSort({ sorts }): void {
    if (this.remotePaging) {
      this.offset = 0;
    }
    if (this.remoteSorting) {
      this.sorters = sorts;
      this.reloadData();
    }
  }

  onPage({ count, pageSize, limit, offset }): void {
    if (this.remotePaging) {
      this.offset = offset;
      this.reloadData();
    }
  }

  onToggleColumn(column: DatatableColumn): void {
    column.hidden = !column.hidden;
    this.saveColumnState();
    this.updateColumns();
  }

  onCopyToClipboard(event: Event, value: any): void {
    event.stopPropagation();
    this.clipboardService.copy(value);
  }

  updateColumns(): void {
    // Load the custom column configuration from the browser
    // local store and filter hidden columns.
    this.loadColumnState();
    this.filteredColumns = this.rawColumns.filter((column) => !column.hidden);
  }

  clearSearchFilter(): void {
    this.searchFilter = '';
    if (!this.remoteSearching) {
      this.rows = [...this.data];
    } else {
      this.reloadData();
    }
  }

  applySearchFilter(): void {
    this.rows = _.filter(this.data, (o) =>
      _.some(this.columns, (column) => {
        let value = _.get(o, column.prop);
        if (!_.isUndefined(column.pipe)) {
          value = column.pipe.transform(value);
        }
        if (value === '' || _.isUndefined(value) || _.isNull(value)) {
          return false;
        }
        if (_.isObjectLike(value)) {
          value = JSON.stringify(value);
        } else if (_.isArray(value)) {
          value = _.join(value, ' ');
        } else if (_.isNumber(value) || _.isBoolean(value)) {
          value = value.toString();
        }
        return _.includes(_.lowerCase(value), _.lowerCase(this.searchFilter));
      })
    );
    this.offset = 0;
  }

  onButtonToggleChange(
    event: MatButtonToggleChange,
    row: Record<string, any>,
    column: DatatableColumn
  ): void {
    const allowNone = _.get(column, 'cellTemplateConfig.allowNone', true);
    // eslint-disable-next-line eqeqeq
    if (allowNone && row[column.prop] == event.value) {
      row[column.prop] = null;
    } else {
      row[column.prop] = event.value;
    }
    this.cellDataChangedEvent.emit({
      value: row[column.prop],
      prop: column.prop,
      row,
      column
    });
  }

  protected initTemplates(): void {
    this.cellTemplates = {
      text: this.textTpl,
      html: this.htmlTpl,
      image: this.imageTpl,
      checkIcon: this.checkIconTpl,
      checkBox: this.checkBoxTpl,
      join: this.joinTpl,
      truncate: this.truncateTpl,
      placeholder: this.placeholderTpl,
      progressBar: this.progressBarTpl,
      notAvailable: this.notAvailableTpl,
      shapeShifter: this.shapeShifterTpl,
      localeDateTime: this.localeDateTimeTpl,
      relativeTime: this.relativeTimeTpl,
      chip: this.chipTpl,
      binaryUnit: this.binaryUnitTpl,
      unsortedList: this.unsortedListTpl,
      template: this.templateTpl,
      buttonToggle: this.buttonToggleTpl,
      copyToClipboard: this.copyToClipboardTpl,
      cronToHuman: this.cronToHumanTpl
    };
  }

  protected sanitizeConfig(): void {
    // Always hide the 'Reload' action button if 'autoReload'
    // is enabled.
    if (this.autoReload) {
      this.hasReloadButton = false;
    }
    this.sanitizeColumns(this.columns);
  }

  /**
   * Sanitize the column configuration, e.g.
   * - convert cellTemplateName => cellTemplate
   * - translate the column headers
   */
  protected sanitizeColumns(columns: DatatableColumn[]): void {
    if (!this.cellTemplates) {
      return;
    }
    columns.forEach((column: DatatableColumn) => {
      column.hidden = !!column.hidden;
      column.sortable = !!column.sortable;
      // Convert column configuration.
      if (_.isString(column.cellTemplateName) && column.cellTemplateName.length) {
        column.cellTemplate = this.cellTemplates[column.cellTemplateName];
      }
      // Translate the column header.
      if (_.isString(column.name) && column.name.length) {
        column.name = translate(column.name);
      }
      // Disable column resizing if mode is `flex`.
      if ('flex' === this.columnMode) {
        column.resizeable = false;
      }
    });
  }

  private loadColumnState(): void {
    if (!this.stateId) {
      return;
    }
    const value = this.userLocalStorageService.get(`datatable_state_${this.stateId}`);
    if (_.isString(value)) {
      const columnsConfig = JSON.parse(value);
      _.forEach(columnsConfig, (columnConfig: Record<string, any>) => {
        const column = _.find(this.columns, ['name', _.get(columnConfig, 'name')]);
        if (column) {
          _.merge(column, columnConfig);
        }
      });
    }
  }

  private saveColumnState(): void {
    if (!this.stateId) {
      return;
    }
    const columnsConfig = [];
    _.forEach(this.columns, (column: DatatableColumn) => {
      columnsConfig.push({
        name: column.name,
        hidden: column.hidden
      });
    });
    this.userLocalStorageService.set(
      `datatable_state_${this.stateId}`,
      JSON.stringify(columnsConfig)
    );
  }
}
