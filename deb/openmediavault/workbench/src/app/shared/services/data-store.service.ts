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
import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import * as _ from 'lodash';
import { EMPTY, Observable, of } from 'rxjs';
import { map } from 'rxjs/operators';

import { formatDeep } from '~/app/functions.helper';
import { Constraint } from '~/app/shared/models/constraint.type';
import { DataStore } from '~/app/shared/models/data-store.type';
import { Sorter } from '~/app/shared/models/sorter.type';
import { ConstraintService } from '~/app/shared/services/constraint.service';
import { RpcService } from '~/app/shared/services/rpc.service';

export type DataStoreResponse = {
  data: Array<any>;
  total: number;
};

@Injectable({
  providedIn: 'root'
})
export class DataStoreService {
  constructor(
    private http: HttpClient,
    private rpcService: RpcService
  ) {}

  /**
   * Load the data. If a proxy is defined, then the RPC is called and the
   * response is stored in the 'data' property.
   *
   * @param store The store to be used.
   */
  load(store: DataStore): Observable<DataStoreResponse> {
    if (_.isPlainObject(store.proxy) && _.has(store.proxy, 'get')) {
      return this.rpcService[store.proxy.get.task ? 'requestTask' : 'request'](
        store.proxy.service,
        store.proxy.get.method,
        store.proxy.get.params
      ).pipe(
        map((res: any) => {
          let total = 0;
          if (_.isPlainObject(res) && _.has(res, 'data')) {
            res = res as DataStoreResponse;
            store.data = res.data;
            total = res.total;
          } else {
            store.data = res;
            total = store.data.length;
          }
          this.onLoad(store);
          return {
            data: store.data,
            total
          };
        })
      );
    } else if (_.isString(store.url)) {
      return this.http.get(store.url).pipe(
        map((res: Record<any, any>) => {
          store.data = res;
          this.onLoad(store);
          return {
            data: store.data,
            total: store.data.length
          };
        })
      );
    } else {
      this.onLoad(store);
      return of({
        data: store.data,
        total: store.data.length
      });
    }
  }

  /**
   * Save the data. If a proxy is defined, then the RPC is called,
   * otherwise nothing is done.
   *
   * @param store The store to be used.
   */
  save(store: DataStore): Observable<any> {
    if (_.isPlainObject(store.proxy) && _.has(store.proxy, 'post')) {
      // Make a deep copy of the data to do not modify due the
      // following operations.
      const data = _.cloneDeep(store.data);
      // Modify every object in the array and pick only the specified
      // properties.
      if (_.isArray(store.proxy.post.filter)) {
        const filterConfig = store.proxy.post.filter;
        _.forEach(data, (item, index) => {
          switch (filterConfig.mode) {
            case 'pick':
              data[index] = _.pick(item, filterConfig.props);
              break;
            case 'omit':
              data[index] = _.omit(item, filterConfig.props);
              break;
          }
        });
      }
      return this.rpcService[store.proxy.post.task ? 'requestTask' : 'request'](
        store.proxy.service,
        store.proxy.post.method,
        data
      );
    } else {
      return EMPTY;
    }
  }

  private onLoad(store: DataStore) {
    // We need to create a new array, otherwise Angular won't
    // detect changes if we modify the original one only.
    let data = [];
    if (_.isPlainObject(store.data)) {
      // Convert simple objects to an array.
      if (_.isUndefined(store.fields) || !_.isArray(store.fields)) {
        store.fields = ['key', 'value'];
      }
      _.forEach(store.data, (value, key) => {
        const newItem = {};
        _.set(newItem, store.fields[0], key);
        _.set(newItem, store.fields[1], value);
        data.push(newItem);
      });
    } else {
      // Iterate over all items and convert/onLoad them from
      // string or array to an object with the specified fields.
      _.forEach(store.data, (item) => {
        let newItem = item;
        if (!_.isPlainObject(item)) {
          newItem = {};
          if (_.isArray(item)) {
            _.forEach(store.fields, (name, index) => {
              const value = _.get(item, index, undefined);
              _.set(newItem, name, value);
            });
          } else {
            _.forEach(store.fields, (name) => {
              _.set(newItem, name, item);
            });
          }
        }
        data.push(newItem);
      });
    }
    // Assign additional sources?
    if (_.isArray(data) && _.isPlainObject(store.assign)) {
      const key = store.assign.key;
      const sources = store.assign.sources;
      _.forEach(data, (item) => {
        const value = _.get(item, key);
        if (_.has(sources, value)) {
          const source = _.get(sources, value);
          _.merge(item, source);
        }
      });
    }
    // Transform data?
    if (_.isPlainObject(store.transform)) {
      _.forEach(data, (item) => {
        const tmp = formatDeep(store.transform, item);
        _.merge(item, tmp);
      });
    }
    // Filter data?
    if (_.isArray(store.filters)) {
      _.forEach(store.filters, (filter: Constraint) => {
        data = ConstraintService.filter(data, filter);
      });
    }
    // Make sure the data is unique.
    if (_.isString(store.uniqBy)) {
      data = _.uniqBy(data, store.uniqBy);
    }
    // Sort data?
    if (_.isArray(store.sorters)) {
      const fields: Array<string> = [];
      const orders: Array<'asc' | 'desc'> = [];
      _.forEach(store.sorters, (sorter: Sorter) => {
        fields.push(sorter.prop);
        orders.push(sorter.dir);
      });
      data = _.orderBy(data, fields, orders);
    }
    store.data = data;
  }
}
