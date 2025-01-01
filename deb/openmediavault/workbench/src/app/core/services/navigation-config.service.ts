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

import { Permissions } from '~/app/shared/models/permissions.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';

// The structure of a menu item in the 'navigation-config.json' asset file.
type MenuItem = {
  // The position of the menu item. This including the text is used
  // to sort the order of the menu items.
  position?: number;
  // The path where to insert and show the menu item, e.g.
  // 'system.certificate.ssl' or 'services.nfs.settings'.
  // The following root paths exists:
  // * diagnostics
  // * network
  // * services
  // * storage
  // * system
  // * usermgmt
  path: string;
  // The menu item text. This is used for sorting the order of the
  // menu items.
  text: string;
  // The icon to be displayed, e.g. the shorthand of an icon listed
  // in 'icon.enums' like 'plus' or 'mdi:xxx' for an icon that is
  // not listed there. See https://materialdesignicons.com/ for the
  // names of the available icons.
  icon?: string;
  // The routing url used to access the page in the browser, e.g.
  // '/services/ssh' or '/system/powermgmt/settings'.
  url: string;
  // The permissions if you only want to show the menu item to
  // specific accounts.
  permissions?: Permissions;
  // Set to `true` to hide the menu.
  hidden?: boolean;
};

// The internal navigation menu structure.
export type NavigationMenuItem = MenuItem & {
  children?: NavigationMenuItem[];
};

@Injectable({
  providedIn: 'root'
})
export class NavigationConfigService {
  public readonly configs$: Observable<NavigationMenuItem[]>;

  private configsSource = new BehaviorSubject<NavigationMenuItem[]>([]);

  constructor(
    private authSessionService: AuthSessionService,
    private http: HttpClient
  ) {
    this.configs$ = this.configsSource.asObservable().pipe(
      map((configs: NavigationMenuItem[]) => {
        const permissions: Permissions = this.authSessionService.getPermissions();
        const filterFn = (items: NavigationMenuItem[]): NavigationMenuItem[] => {
          const result = [];
          items.forEach((item: NavigationMenuItem) => {
            if (item.hidden === true) {
              return;
            }
            if (_.has(item, 'permissions')) {
              // Check if the current user has the permission to view this
              // menu item.
              if (!Permissions.validate(item.permissions, permissions)) {
                return;
              }
            }
            const newItem = _.clone(item);
            if (_.has(item, 'children') && _.isArray(item.children)) {
              newItem.children = filterFn(item.children);
            }
            result.push(newItem);
          });
          return result;
        };
        return filterFn(configs);
      })
    );
  }

  public load(): Observable<NavigationMenuItem[]> {
    /**
     * Load the navigation asset file and build the navigation configuration
     * used by the `NavigationBarComponent` and `NavigationPageComponent` components.
     */
    return this.http.get('./assets/navigation-config.json').pipe(
      catchError((error) => {
        error.preventDefault?.();
        return of([]);
      }),
      map((menuItemConfigs: Array<MenuItem>) => {
        const pushMenuItem = (
          collection: Array<NavigationMenuItem>,
          menuItemConfig: MenuItem,
          parentPath: string = undefined
        ) => {
          const pathParts = _.filter(
            _.split(_.trim(menuItemConfig.path, '.').replace(parentPath, ''), '.'),
            (value) => !_.isEmpty(value)
          );
          let currentPath = `${pathParts.shift()}`;
          if (parentPath) {
            currentPath = `${parentPath}.${currentPath}`;
          }

          let element = _.find(collection, ['path', currentPath]);
          if (_.isUndefined(element)) {
            // Create a temporary placeholder menu item. This should be
            // configured correctly sometime while processing all navigation
            // menus.
            element = {
              position: 0,
              path: currentPath,
              text: '???',
              url: '/404'
            };
            collection.push(element);
          }

          if (pathParts.length) {
            const children = _.get(element, 'children', []);
            pushMenuItem(children, menuItemConfig, currentPath);
            element.children = children;
          } else {
            element.position = menuItemConfig.position;
            element.text = menuItemConfig.text;
            element.url = menuItemConfig.url;
            if (menuItemConfig.icon) {
              element.icon = menuItemConfig.icon;
            }
            if (menuItemConfig.permissions) {
              element.permissions = menuItemConfig.permissions;
            }
            element.hidden = _.defaultTo(menuItemConfig.hidden, false);
          }

          // Sort the menu items.
          const sorted = _.sortBy(collection, ['position', 'text']);
          collection.length = 0;
          collection.push(...sorted);
        };

        const navMenuItems: Array<NavigationMenuItem> = [];
        _.forEach(menuItemConfigs, (menuItemConfig: MenuItem) => {
          pushMenuItem(navMenuItems, menuItemConfig);
        });
        return navMenuItems;
      }),
      tap((config: Array<NavigationMenuItem>) => {
        this.configsSource.next(config);
      })
    );
  }

  public getChildrenByUrl(url): Observable<NavigationMenuItem[]> {
    return this.configs$.pipe(
      map((configs: NavigationMenuItem[]) => {
        const filterFn = (items: NavigationMenuItem[]): NavigationMenuItem[] => {
          const result = [];
          items.forEach((item: NavigationMenuItem) => {
            if (item.url === url && _.isArray(item.children) && !item.hidden) {
              result.push(...item.children);
            } else if (_.startsWith(url, `${item.url}/`) && _.isArray(item.children)) {
              result.push(...filterFn(item.children));
            }
          });
          return result;
        };
        if ('/' === url) {
          return configs;
        }
        return filterFn(configs);
      })
    );
  }
}
