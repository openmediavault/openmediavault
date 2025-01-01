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
import { BehaviorSubject, Observable, of } from 'rxjs';
import { catchError, map, tap } from 'rxjs/operators';

import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { Permissions, Roles } from '~/app/shared/models/permissions.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

@Injectable({
  providedIn: 'root'
})
export class DashboardWidgetConfigService {
  public readonly configs$: Observable<DashboardWidgetConfig[]>;

  private configsSource = new BehaviorSubject<DashboardWidgetConfig[]>([]);

  constructor(
    private authSessionService: AuthSessionService,
    private http: HttpClient,
    private userLocalStorageService: UserLocalStorageService
  ) {
    this.configs$ = this.configsSource.asObservable();
  }

  /**
   * Load the dashboard widget configuration. Widgets that require more
   * permissions than the current user owns are automatically filtered out.
   */
  public load(): Observable<DashboardWidgetConfig[]> {
    return this.http.get('./assets/dashboard-widget-config.json').pipe(
      catchError((error) => {
        error.preventDefault?.();
        return of([]);
      }),
      map((widgets: Array<DashboardWidgetConfig>) => {
        const result: Array<DashboardWidgetConfig> = [];
        _.forEach(widgets, (widget: DashboardWidgetConfig) => {
          // Sanitize the widget configuration.
          widget.permissions = Permissions.fromObject(
            widget.permissions ? widget.permissions : { role: _.values(Roles) }
          );
          // Append the widget if the privileges of the logged in user
          // match those of the widget.
          if (Permissions.validate(widget.permissions, this.authSessionService.getPermissions())) {
            result.push(widget);
          }
        });
        return result;
      }),
      tap((config: Array<DashboardWidgetConfig>) => {
        this.configsSource.next(config);
      })
    );
  }

  /**
   * Get the identifiers of all enabled widgets of the current user.
   *
   * @return Returns a list of widget identifiers (UUID).
   */
  public getEnabled(): Array<string> {
    const value = this.userLocalStorageService.get('dashboard_widgets', '[]');
    const result: Array<string> = JSON.parse(value);
    return result;
  }

  /**
   * Set the identifiers of all enabled widgets for the current user.
   *
   * @param ids The list of identifiers of the enabled widgets.
   */
  public setEnabled(ids: Array<string>) {
    const value = JSON.stringify(ids);
    this.userLocalStorageService.set('dashboard_widgets', value);
  }
}
