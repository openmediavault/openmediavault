import { Constraint } from '~/app/shared/models/constraint.type';
import { Sorter } from '~/app/shared/models/sorter.type';

export type DataStore = {
  // An array of data to load inline. If the data is an dict/object,
  // then it will be auto-converted into an array with the specified
  // fields (defaults to 'key' and 'value').
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
      // Set `true` if the RPC is a long running background task.
      task?: boolean;
    };
    // The name of the RPC to update the data.
    post?: {
      // The name of the RPC.
      method: string;
      // The RPC parameters.
      params?: any;
      // Set `true` if the RPC is a long running background task.
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
  // Transform the data. The given fields can be tokenized strings
  // that will be formatted using the origin data. Finally these fields
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
};
