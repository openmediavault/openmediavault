/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
  labelColor?: string;
  backgroundColor: string;
};

export type DashboardWidgetConfig = {
  id: string; // An UUID.
  // The following widgets are general purpose and customizable:
  // - datatable
  // - rrd
  // - chart
  // All other widget types are internal only.
  type:
    | 'datatable'
    | 'rrd'
    | 'chart'
    | 'system-information' // internal
    | 'services-status' // internal
    | 'filesystems-status' // internal
    | 'smart-status'; // internal
  title: string;
  permissions?: Permissions;
  // The frequency in milliseconds with which the data
  // should be reloaded. Defaults are:
  // - datatable: 10000
  // - rrd: 60000
  // - chart: 10000
  reloadPeriod?: number;

  datatable?: {
    columns: Array<DatatableColumn>;
    columnMode?: 'standard' | 'flex' | 'force';
    sorters?: Array<Sorter>;
    store: DataStore;
    hasHeader?: boolean; // Defaults to `true`.
    hasFooter?: boolean; // Defaults to `false`.
  };

  rrd?: {
    name: string;
  };

  chart?: {
    type: 'doughnut' | 'advanced-doughnut' | 'gauge' | 'advanced-gauge';
    label?: {
      display?: boolean;
      formatter: 'none' | 'label' | 'template';
      formatterConfig?: any;
      color: string;
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
      // Transform the data. The given fields can be tokenized strings
      // that will be formatted using the origin data. Finally these fields
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
};
