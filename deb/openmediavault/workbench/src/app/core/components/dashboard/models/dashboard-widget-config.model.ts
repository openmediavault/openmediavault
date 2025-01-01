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
import { DataStore } from '~/app/shared/models/data-store.type';
import { DatatableColumn } from '~/app/shared/models/datatable-column.type';
import { Permissions } from '~/app/shared/models/permissions.model';
import { Sorter } from '~/app/shared/models/sorter.type';

export type ChartDataConfig = {
  label: string;
  prop: string;
  labelColor?: string; // In hexadecimal format.
  backgroundColor: string; // In hexadecimal format.
};

export type DashboardWidgetConfig = {
  id: string; // A unique UUID.
  // The following widgets are general purpose and customizable:
  // - chart
  // - datatable
  // - grid
  // - rrd
  // - text
  // - value
  // All other widget types are internal only.
  type:
    | 'chart'
    | 'datatable'
    | 'grid'
    | 'rrd'
    | 'text'
    | 'value'
    | 'system-information' // internal
    | 'filesystems-status'; // internal
  // The widget title.
  title: string;
  hideTitle?: boolean;
  // The description of the widget.
  description?: string;
  permissions?: Permissions;
  // The frequency in milliseconds with which the data
  // should be reloaded. Defaults are:
  // - chart: 10000
  // - datatable: 10000
  // - grid: 10000
  // - rrd: 60000
  // - text: 10000
  // - value: 10000
  reloadPeriod?: number;

  grid?: {
    item: {
      // Custom CSS class. This can be a template, too.
      class?: string;
      // The template that generates the grid item title.
      title?: string;
      titleClass?: string;
      // The template that generates the grid item content.
      content?: string;
      contentClass?: string;
      // The template to generate the grid item tooltip text.
      tooltip?: string;
      // The template to generate the URL to navigate to when
      // a grid item is clicked.
      url?: string;
      // The minimum width of an item. Defaults to `100px`.
      minWidth?: string;
    };
    // The empty message. Defaults to `No data to display`.
    emptyMessage?: string;
    // Show an empty message if there is no data. Defaults to `true`.
    hasEmptyMessage?: boolean;
    // The data to be displayed.
    store: DataStore;
  };

  datatable?: {
    columns: Array<DatatableColumn>;
    columnMode?: 'standard' | 'flex' | 'force';
    sorters?: Array<Sorter>;
    sortType?: 'single' | 'multi';
    // The data to be displayed.
    store: DataStore;
    hasHeader?: boolean; // Defaults to `true`.
    hasFooter?: boolean; // Defaults to `false`.
  };

  rrd?: {
    kind: string;
    period: 'hour' | 'day' | 'week' | 'month' | 'year';
  };

  chart?: {
    type: 'doughnut' | 'advanced-doughnut' | 'gauge' | 'advanced-gauge';
    label?: {
      display?: boolean;
      formatter: 'none' | 'label' | 'template';
      formatterConfig?: any;
      color: string; // In hexadecimal format.
      align?: 'bottom' | 'center' | 'end' | 'left' | 'right' | 'start' | 'top' | number;
      anchor?: 'center' | 'end' | 'start';
    };
    legend?: {
      display?: boolean;
      position?: 'left' | 'right' | 'top' | 'bottom' | 'chartArea';
    };
    dataConfig: Array<ChartDataConfig>;
    tooltips?: boolean;
    // Max. height/width of the chart, e.g. `150px`.
    maxHeight?: string;
    maxWidth?: string;
    // In degrees. Defaults to 360.
    circumference?: number;
    // In degrees. Defaults to 0.
    rotation?: number;
    request?: {
      service: string;
      method: string;
      params?: Record<string, any>;
      // Set `true` if the RPC is a long-running background task.
      task?: boolean;
      // Transform the data. The given fields can be tokenized strings
      // that will be formatted using the origin data. Finally, these fields
      // are merged with the origin data.
      // Example:
      // Response = [{ foo: 'bar', num: '3' }]
      // transform = { foo: '{{ foo }} xxx', num: '{{ num | int }}', add: 'foo' }
      // Result = [{ foo: 'bar xxx', num: 3, add: 'foo' }]
      transform?: Record<string, string>;
    };

    // --- gauge | advanced-gauge ---
    min?: number;
    max?: number;
    displayValue?: boolean;
  };

  text?: {
    // Custom CSS class.
    class?: string;
    request?: {
      service: string;
      method: string;
      params?: Record<string, any>;
      // Set `true` if the RPC is a long-running background task.
      task?: boolean;
    };
  };

  value?: {
    icon?: string;
    // The string or template to render the title.
    title: string;
    // The string or template to render the value.
    value: string;
    // The template to generate the URL to navigate to when
    // the widget is clicked.
    url?: string;
    request?: {
      service: string;
      method: string;
      params?: Record<string, any>;
      // Set `true` if the RPC is a long-running background task.
      task?: boolean;
    };
  };
};
