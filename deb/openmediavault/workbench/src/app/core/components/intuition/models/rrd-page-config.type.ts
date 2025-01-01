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

export type RrdPageGraphConfig = {
  // The label of the tab.
  label?: string;
  // The name of the RRD PNG image, e.g. 'cpu-0' or 'uptime'.
  name: string;
};

export type RrdPageConfig = {
  // The RRD graphs rendered per tab. If only one graph is configured,
  // no tab page is display, hence the `label` property is not needed.
  graphs: Array<RrdPageGraphConfig>;
  // If a store is defined, then a top tab is displayed with the number
  // of tabs as items are in the store.
  // The 'name' and 'label' properties in the `graphs` configuration
  // will be formatted using the data in the store, thus make sure the
  // store data is an array of objects.
  store?: DataStore;
  // The label used in the top tab headers. This can be a tokenized
  // string which is formatted using the store items. Note, this
  // property is only used when a store is defined.
  // Example:
  // store.data = [{name: 'foo', x: 0}, {name: 'bar', x: 2}]
  // label = '{{ name }}'
  // This will result in two tabs with the headers 'foo' and 'bar'.
  label?: string;
};
