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
import { Injectable, Type } from '@angular/core';
import { Route, Routes } from '@angular/router';
import * as _ from 'lodash';
import { Observable, of, ReplaySubject } from 'rxjs';
import { catchError, map } from 'rxjs/operators';

import { CodeEditorPageComponent } from '~/app/core/components/intuition/code-editor-page/code-editor-page.component';
import { DatatablePageComponent } from '~/app/core/components/intuition/datatable-page/datatable-page.component';
import { FormPageComponent } from '~/app/core/components/intuition/form-page/form-page.component';
import { RrdPageComponent } from '~/app/core/components/intuition/rrd-page/rrd-page.component';
import { SelectionListPageComponent } from '~/app/core/components/intuition/selection-list-page/selection-list-page.component';
import { TabsPageComponent } from '~/app/core/components/intuition/tabs-page/tabs-page.component';
import { TextPageComponent } from '~/app/core/components/intuition/text-page/text-page.component';
import { BlankPageComponent } from '~/app/core/pages/blank-page/blank-page.component';
import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';
import { IsDirtyGuardService } from '~/app/shared/services/is-dirty-guard.service';

const componentMap: Record<string, Type<any>> = {
  blankPage: BlankPageComponent,
  navigationPage: NavigationPageComponent,
  formPage: FormPageComponent,
  selectionListPage: SelectionListPageComponent,
  textPage: TextPageComponent,
  tabsPage: TabsPageComponent,
  datatablePage: DatatablePageComponent,
  rrdPage: RrdPageComponent,
  codeEditorPage: CodeEditorPageComponent
};

type RouteConfig = {
  url: string;
  title?: string;
  breadcrumb?: {
    text: string;
    request?: {
      service: string;
      method: string;
      params?: Record<string, any>;
    };
  };
  editing?: boolean;
  notificationTitle?: string;
  component: {
    type:
      | 'blankPage'
      | 'navigationPage'
      | 'formPage'
      | 'selectionListPage'
      | 'textPage'
      | 'tabsPage'
      | 'datatablePage'
      | 'rrdPage'
      | 'codeEditorPage';
    config: Record<string, any>;
  };
};

const getSegments = (path: string): Array<string> => {
  return _.split(_.trim(path, '/'), '/');
};

@Injectable({
  providedIn: 'root'
})
export class RouteConfigService {
  public readonly configs$: Observable<Routes>;

  private configsSource = new ReplaySubject<Routes>(1);

  constructor(private http: HttpClient) {
    this.configs$ = this.configsSource.asObservable();
  }

  public load(): Observable<Routes> {
    return this.http.get('./assets/route-config.json').pipe(
      catchError((error) => {
        error.preventDefault?.();
        return of([]);
      }),
      map((configs: Array<RouteConfig>) => {
        const routes: Routes = [];
        // Convert the loaded route configuration into Angular
        // 'Route' objects.
        _.forEach(configs, (config) => {
          const route: Route = {
            path: config.url,
            component: componentMap[config.component.type],
            data: {
              title: config.title,
              breadcrumb: config.breadcrumb,
              editing: config.editing,
              notificationTitle: config.notificationTitle,
              config: config.component.config
            }
          };
          switch (config.component.type) {
            case 'formPage':
            case 'selectionListPage':
              _.merge(route, {
                canDeactivate: [IsDirtyGuardService]
              });
              break;
          }
          routes.push(route);
        });
        this.configsSource.next(routes);
        return routes;
      })
    );
  }

  /**
   * @param rootSegment The name of the root segment, this can be 'diagnostics',
   *   'network', 'services', 'storage', 'system' or 'usermgmt'.
   * @param targetNode The root node where to add the custom routes.
   */
  public inject(rootSegment: string, targetNode: Routes): void {
    this.configs$.subscribe((customRoutes) => {
      // Get the custom routes to inject at the given node. Sort the
      // filtered routes by the number of segments, thus new child
      // nodes are created in the correct order.
      const filteredRoutes: Routes = _.sortBy(
        _.filter(customRoutes, (customRoute: Route) => {
          const segments = getSegments(customRoute.path);
          return segments[0] === rootSegment;
        }),
        (customRoute: Route) => {
          const segments = getSegments(customRoute.path);
          return segments;
        }
      );
      _.forEach(filteredRoutes, (filteredRoute: Route) => {
        // eslint-disable-next-line no-shadow
        const getParentNode = (routes: Routes, segments: Array<string>): Routes => {
          if (!segments.length) {
            return routes;
          }
          const segment: string = segments.shift();
          let node = _.find(routes, (route: Route) => route.path === segment);
          if (_.isUndefined(node)) {
            // Create the missing node. Use the URL segment name as
            // default title.
            node = {
              path: segment,
              data: {
                title: _.upperFirst(segment)
              },
              children: []
            };
            routes.push(node);
          }
          if (_.isArray(node.children) && segments.length) {
            return getParentNode(node.children, segments);
          }
          if (!node.children) {
            // If we are here then the requested path does not exist
            // right now and the node is a leaf.
            node.children = [];
            // If there is a configured component, then we need to
            // rebuild the route configuration.
            // If no component exists, then append the 'NavigationPageComponent'
            // by default.
            if (node.component) {
              node.children.push({
                path: '',
                component: node.component,
                data: _.pick(node.data, ['config']),
                canDeactivate: node.canDeactivate
              });
              node.data = _.pick(node.data, ['title']);
              delete node.component;
            } else {
              node.children.push({
                path: '',
                component: NavigationPageComponent
              });
            }
          }
          return node.children;
        };

        // Process the path.
        const routeSegments: Array<string> = getSegments(filteredRoute.path);
        // Remove the first segment.
        routeSegments.shift();
        // Remove the last segment.
        let childPath: string = routeSegments.pop();
        if (_.startsWith(childPath, ':')) {
          // Process paths like 'edit/:name'.
          childPath = [routeSegments.pop(), childPath].join('/');
        }

        const parentNode = getParentNode(targetNode, routeSegments);
        if (!_.isUndefined(parentNode)) {
          const childNode = _.find(parentNode, ['path', childPath]);
          if (childNode) {
            childNode.data = { title: filteredRoute.data.title };
            childNode.children.push(
              _.merge(
                { path: '', data: { config: filteredRoute.data.config } },
                _.omit(filteredRoute, 'data', 'path')
              )
            );
          } else {
            parentNode.push(_.merge({ path: childPath }, _.omit(filteredRoute, 'path')));
          }
        }
      });
    });
  }
}
