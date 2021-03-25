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
  public configs$: Observable<ILogConfig[]>;

  private configsSource = new ReplaySubject<ILogConfig[]>(1);

  constructor(private http: HttpClient) {
    this.configs$ = this.configsSource.asObservable();
  }

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
