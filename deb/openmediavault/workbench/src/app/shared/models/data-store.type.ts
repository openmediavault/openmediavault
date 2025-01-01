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
import { Constraint } from '~/app/shared/models/constraint.type';
import { Sorter } from '~/app/shared/models/sorter.type';

export type DataStore = {
  // An array of data to load inline. If the data is a dict/object,
  // then it will be auto-converted into an array with the specified
  // fields (defaults to ['key', 'value']).
  data?: any;
  // The field names of an item in 'data'. This is only
  // necessary when loading inline data, but not when a
  // proxy is configured to load the data via RPC.
  fields?: Array<string>;
  // The name of the field treated as unique id.
  idField?: string;
  // A proxy is used to handle the loading of data.
  // The RPC response is stored in the 'data' field.
  proxy?: {
    // The name of the RPC service.
    service: string;
    // The RPC to get the data.
    get: {
      // The name of the RPC.
      method: string;
      // The RPC parameters.
      params?: Record<string, any>;
      // Set `true` if the RPC is a long-running background task.
      task?: boolean;
    };
    // The name of the RPC to update the data.
    post?: {
      // The name of the RPC.
      method: string;
      // The RPC parameters.
      params?: any;
      // Set `true` if the RPC is a long-running background task.
      task?: boolean;
      // Filter the specified properties for every object in the
      // data store before the request is executed.
      filter?: {
        mode: 'pick' | 'omit';
        props: Array<string>;
      };
    };
  };
  // Load a file via HTTP.
  url?: string;
  // Assigns additional sources to objects within the data that are
  // identified by the specified key.
  // Example:
  // Response = ['foo', 'bar] <- will be converted automatically to an object.
  // assignByKey = { key: 'value', sources: { foo: { text: 'xyz' } } }
  // Result = [{ key: 'foo', value: 'foo', text: 'xyz' }, { key: 'bar', value: 'bar' }]
  assign?: { key: string; sources: Record<string, any> };
  // Transform the data. The given fields can be tokenized strings
  // that will be formatted using the origin data. Finally, these fields
  // are merged with the origin data.
  // Example:
  // Response = [{ foo: 'bar', num: '3' }]
  // transform = { foo: '{{ foo }} xxx', num: '{{ num | int }}', add: 'foo' }
  // Result = [{ foo: 'bar xxx', num: 3, add: 'foo' }]
  transform?: Record<string, string>;
  // Sort the local data according the specified sorters.
  sorters?: Array<Sorter>;
  // Filter that data that fulfills the specified constraint.
  filters?: Array<Constraint>;
  // Make sure the data contains only unique values of a certain
  // property.
  uniqBy?: string;
};
