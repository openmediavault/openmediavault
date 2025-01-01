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
import { Observable, of, ReplaySubject } from 'rxjs';
import { catchError, tap } from 'rxjs/operators';

import { DatatableColumn } from '~/app/shared/models/datatable-column.type';

export type LogConfig = {
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
};

@Injectable({
  providedIn: 'root'
})
export class LogConfigService {
  public readonly configs$: Observable<LogConfig[]>;

  private configsSource = new ReplaySubject<LogConfig[]>(1);

  constructor(private http: HttpClient) {
    this.configs$ = this.configsSource.asObservable();
  }

  public load(): Observable<LogConfig[]> {
    return this.http.get('./assets/log-config.json').pipe(
      catchError((error) => {
        error.preventDefault?.();
        return of([]);
      }),
      tap((logs: Array<LogConfig>) => {
        this.configsSource.next(logs);
      })
    );
  }
}
