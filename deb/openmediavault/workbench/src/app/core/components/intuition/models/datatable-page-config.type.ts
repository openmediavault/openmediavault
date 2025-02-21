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
import { DatatablePageActionConfig } from '~/app/core/components/intuition/models/datatable-page-action-config.type';
import { PageHintConfig } from '~/app/core/components/intuition/models/page-config.type';
import { DataStore } from '~/app/shared/models/data-store.type';
import { DatatableColumn } from '~/app/shared/models/datatable-column.type';
import { Sorter } from '~/app/shared/models/sorter.type';

export type DatatablePageConfig = {
  // A list of hints to be displayed at the top of the page.
  hints?: Array<PageHintConfig>;
  // A title within the datatable header.
  title?: string;
  // A subtitle within the datatable header.
  subTitle?: string;
  // An image used as an avatar within the datatable header.
  icon?: string;
  // An identifier which identifies this datatable uniquely.
  // This is used to store/restore the column state.
  stateId?: string;
  // The name of the property that identifies a row uniquely.
  rowId?: string;
  // The format string that is used to generate a human-readable
  // identifier of a row. This is used, for example, in the delete
  // dialog to enumerate the selected row(s).
  rowEnumFmt?: string;
  // The column configuration.
  columns: Array<DatatableColumn>;
  columnMode?: 'standard' | 'flex' | 'force';
  hasActionBar?: boolean;
  hasSearchField?: boolean;
  hasHeader?: boolean;
  hasFooter?: boolean;
  selectionType?: 'none' | 'single' | 'multi';
  updateSelectionOnReload?: 'always' | 'onChange' | 'never';
  // Page size to show. To disable paging, set the limit to 0.
  // Defaults to 25.
  limit?: number;
  // If set to `true`, the parameters `start` and `limit` from the data
  // table paging component are added to the RPC request parameters.
  // Defaults to `false`.
  remotePaging?: boolean;
  // If set to `true`, the sorting order and property name of the current
  // active column of the data table are added as parameters `sortdir` and
  // `sortfield` to the RPC request parameters. Defaults to `false`.
  remoteSorting?: boolean;
  // If set to `true`, the content of the search field of the data table
  // is added as parameter `search` to the RPC request parameters.
  // Defaults to `false`.
  remoteSearching?: boolean;
  // Automatically load the data after datatable has been initialized.
  // Defaults to `true`.
  autoLoad?: boolean;
  // The frequency in milliseconds with which the data should be reloaded.
  // Defaults to `false`.
  autoReload?: boolean | number;
  sorters?: Array<Sorter>;
  sortType?: 'single' | 'multi';
  store?: DataStore;
  actions?: Array<DatatablePageActionConfig>;
  // The page footer buttons.
  buttonAlign?: 'start' | 'center' | 'end';
  buttons?: Array<DatatablePageButtonConfig>;
};

export type DatatablePageButtonConfig = {
  // Specifies a template that pre-configures the button:
  // back   - A button with the text 'Back'.
  // cancel - A button with the text 'Cancel'.
  // submit - A button with the text 'Save'.
  template?: 'back' | 'cancel' | 'submit';
  // The text displayed in the button.
  text?: string;
  // Custom CSS class.
  class?: string;
  // Disable the button. Defaults to 'false'.
  disabled?: boolean;
  // The URL of the route to navigate to when the button has been
  // clicked.
  // Both options are mutually exclusive.
  url?: string;
  // A callback function that is called when the button has been
  // clicked. Internal only.
  click?: (buttonConfig: DatatablePageButtonConfig, store: DataStore) => void;
};
