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
