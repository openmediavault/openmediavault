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
import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import * as _ from 'lodash';
import { Observable, of, ReplaySubject } from 'rxjs';
import { catchError, tap } from 'rxjs/operators';

import { DatatableColumn } from '~/app/shared/models/datatable-column.type';

export interface ILogConfig {
  // A unique identifier.
  id: string;
  text: string;
  request?: {
    service?: string;
    method?: string;
    params?: Record<string, any>;
  };
  // The datatable columns.
  columns: Array<DatatableColumn>;
  // The datatable sorters.
  sorters?: Array<{
    dir: 'asc' | 'desc';
    prop: string;
  }>;
}

@Injectable({
  providedIn: 'root'
})
export class LogConfigService {
  private configsSource = new ReplaySubject<ILogConfig[]>(1);

  // eslint-disable-next-line @typescript-eslint/member-ordering
  public readonly configs$: Observable<ILogConfig[]> = this.configsSource.asObservable();

  constructor(private http: HttpClient) {}

  public load(): Observable<ILogConfig[]> {
    return this.http.get('./assets/log-config.json').pipe(
      catchError((error) => {
        if (_.isFunction(error.preventDefault)) {
          error.preventDefault();
        }
        return of([]);
      }),
      tap((logs: Array<ILogConfig>) => {
        this.configsSource.next(logs);
      })
    );
  }
}
