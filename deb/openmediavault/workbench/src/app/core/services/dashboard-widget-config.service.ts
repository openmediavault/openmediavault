/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2023 Volker Theile
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
import { Platform } from '@angular/cdk/platform';
import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import * as _ from 'lodash';
import { BehaviorSubject, Observable, of } from 'rxjs';
import { catchError, map, mergeMap, take, tap } from 'rxjs/operators';

import { DashboardUserConfig } from '~/app/core/components/dashboard/models/dashboard-user-config.model';
import { DashboardWidgetConfig } from '~/app/core/components/dashboard/models/dashboard-widget-config.model';
import { Permissions, Roles } from '~/app/shared/models/permissions.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { RpcService } from '~/app/shared/services/rpc.service';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

@Injectable({
  providedIn: 'root'
})
export class DashboardWidgetConfigService {
  public readonly configs$: Observable<DashboardWidgetConfig[]>;

  private configsSource = new BehaviorSubject<DashboardWidgetConfig[]>([]);

  private get platformDeviceType(): string {
    return this.platform.ANDROID || this.platform.IOS ? 'mobile' : 'desktop';
  }

  constructor(
    private authSessionService: AuthSessionService,
    private http: HttpClient,
    private userLocalStorageService: UserLocalStorageService,
    public platform: Platform,
    private rpcService: RpcService
  ) {
    this.configs$ = this.configsSource.asObservable();
  }

  /**
   * Load the dashboard widget configuration. Widgets that require more
   * permissions than the current user owns are automatically filtered out.
   */
  public loadWidgetConfig(): Observable<DashboardWidgetConfig[]> {
    return this.http.get('./assets/dashboard-widget-config.json').pipe(
      catchError((error) => {
        if (_.isFunction(error.preventDefault)) {
          error.preventDefault();
        }
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
   * Load the user's dashboard widget configuration.
   */
  public loadUserWidgetConfig(): Observable<DashboardUserConfig> {
    return this.rpcService
      .request('Dashboard', 'getDashboardSetting', { deviceType: this.platformDeviceType })
      .pipe(
        catchError((error) => {
          if (_.isFunction(error.preventDefault)) {
            error.preventDefault();
          }
          return of(undefined);
        }),
        take(1),
        mergeMap((userWidgetConfig: DashboardUserConfig) => {
          const emptyConfig: DashboardUserConfig = { widgets: [] };
          if (userWidgetConfig !== undefined) {
            if (userWidgetConfig == null) {
              const enabledLocal = this.getEnabledLocal();
              if (enabledLocal.length > 0) {
                return this.setEnabled(enabledLocal).pipe(take(1));
              } else {
                return of(emptyConfig);
              }
            } else {
              this.setEnabledLocal(userWidgetConfig);
              return of(userWidgetConfig);
            }
          } else {
            return of(emptyConfig);
          }
        })
      );
  }

  /**
   * Get the identifiers of all enabled widgets of the current user.
   *
   * @return Returns a list of widget identifiers (UUID).
   */
  public getEnabled(): Observable<DashboardUserConfig> {
    const value = this.userLocalStorageService.get('dashboard_widgets', '{"widgets":[]}');
    const result: DashboardUserConfig = JSON.parse(value);
    return of('widgets' in result ? result : { widgets: [] });
  }

  /**
   * Set the identifiers of all enabled widgets for the current user.
   *
   * @param ids The list of identifiers of the enabled widgets.
   *
   * @return Returns an observable of the DashboardUserConfig containing a list of widgets
   */
  public setEnabled(enabled: Array<string>): Observable<DashboardUserConfig> {
    return this.setEnabledRemote(enabled).pipe(
      tap((result) => this.setEnabledLocal(result)),
      take(1)
    );
  }

  /**
   * Resets the dashboard configuration
   */
  public reset(): Observable<void> {
    return this.resetRemote().pipe(tap(() => this.resetLocal()));
  }

  /**
   * Set the identifiers of all enabled widgets for the current user.
   *
   * @param enabled The list of identifiers of the enabled widgets.
   *
   * @return Returns an observable of the DashboardUserConfig containing a list of widgets
   */
  private setEnabledRemote(enabled: Array<string>): Observable<DashboardUserConfig> {
    const userWidgetConfig: DashboardUserConfig = {
      widgets: enabled.map((m) => {
        return { id: m };
      })
    };
    return this.rpcService.request('Dashboard', 'setDashboardSetting', {
      deviceType: this.platformDeviceType,
      data: userWidgetConfig
    });
  }

  /**
   * Deletes the remote dashboard configuration
   */
  private resetRemote() {
    return this.rpcService
      .request('Dashboard', 'deleteDashboardSetting', { deviceType: this.platformDeviceType })
      .pipe(take(1));
  }

  /**
   * Get the identifiers of all enabled widgets of the current user stored locally.
   *
   * @return Returns a list of widget identifiers (UUID).
   */
  private getEnabledLocal(): Array<string> {
    const value = this.userLocalStorageService.get('dashboard_widgets', '[]');
    const result: Array<string> = JSON.parse(value);
    return result;
  }

  /**
   * Set the identifiers of all enabled widgets for the current user stored locally.
   *
   * @param userConfig The DashboardUserConfig containing a list of widgets
   */
  private setEnabledLocal(userConfig: DashboardUserConfig) {
    this.userLocalStorageService.set('dashboard_widgets', JSON.stringify(userConfig));
  }

  /**
   * Deletes the local dashboard configuration
   */
  private resetLocal() {
    this.userLocalStorageService.remove('dashboard_widgets');
  }
}
