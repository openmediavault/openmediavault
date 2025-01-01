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

export type MkfsConfig = {
  text: string;
  url: string;
};

@Injectable({
  providedIn: 'root'
})
export class MkfsConfigService {
  public readonly configs$: Observable<MkfsConfig[]>;

  private configsSource = new ReplaySubject<MkfsConfig[]>(1);

  constructor(private http: HttpClient) {
    this.configs$ = this.configsSource.asObservable();
  }

  public load(): Observable<MkfsConfig[]> {
    return this.http.get('./assets/mkfs-config.json').pipe(
      catchError((error) => {
        error.preventDefault?.();
        return of([]);
      }),
      tap((logs: Array<MkfsConfig>) => {
        this.configsSource.next(logs);
      })
    );
  }
}
