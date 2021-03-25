import { DatatablePageActionConfig } from '~/app/core/components/limn-ui/models/datatable-page-action-config.type';
import { DataStore } from '~/app/shared/models/data-store.type';
import { DatatableColumn } from '~/app/shared/models/datatable-column.type';
import { Sorter } from '~/app/shared/models/sorter.type';

export type DatatablePageConfig = {
  // A title within the header.
  title?: string;
  // A subtitle within the header.
  subTitle?: string;
  // An image used as an avatar within the header.
  icon?: string;
  // An identifier which identifies this datatable uniquely.
  // This is used to store/restore the column state.
  stateId?: string;
  // The name of the property that identifies a row uniquely.
  rowId?: string;
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
  limit?: number;
  remotePaging?: boolean;
  remoteSorting?: boolean;
  // Automatically load the data after datatable has been initialized.
  autoLoad?: boolean;
  // The frequency in milliseconds with which the data should be reloaded.
  autoReload?: boolean | number;
  sorters?: Array<Sorter>;
  store?: DataStore;
  actions?: Array<DatatablePageActionConfig>;
  // The page footer buttons.
  buttonAlign?: 'left' | 'center' | 'right';
  buttons?: Array<DatatablePageButtonConfig>;
};

export type DatatablePageButtonConfig = {
  // The text displayed in the button.
  text: string;
  // Custom CSS class.
  class?: string;
  // The URL of the route to navigate to when the button has been
  // clicked.
  // Both options are mutually exclusive.
  url?: string;
  // A callback function that is called when the button has been
  // clicked. Internal only.
  click?: (buttonConfig: DatatablePageButtonConfig, store: DataStore) => void;
};
