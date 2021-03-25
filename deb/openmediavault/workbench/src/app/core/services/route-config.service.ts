import { HttpClient } from '@angular/common/http';
import { Injectable, Type } from '@angular/core';
import { Route, Routes } from '@angular/router';
import * as _ from 'lodash';
import { Observable, of, ReplaySubject } from 'rxjs';
import { catchError, map } from 'rxjs/operators';

import { DatatablePageComponent } from '~/app/core/components/limn-ui/datatable-page/datatable-page.component';
import { FormPageComponent } from '~/app/core/components/limn-ui/form-page/form-page.component';
import { RrdPageComponent } from '~/app/core/components/limn-ui/rrd-page/rrd-page.component';
import { SelectionListPageComponent } from '~/app/core/components/limn-ui/selection-list-page/selection-list-page.component';
import { TabsPageComponent } from '~/app/core/components/limn-ui/tabs-page/tabs-page.component';
import { TextPageComponent } from '~/app/core/components/limn-ui/text-page/text-page.component';
import { NavigationPageComponent } from '~/app/core/pages/navigation-page/navigation-page.component';

const componentMap: Record<string, Type<any>> = {
  navigationPage: NavigationPageComponent,
  formPage: FormPageComponent,
  selectionListPage: SelectionListPageComponent,
  textPage: TextPageComponent,
  tabsPage: TabsPageComponent,
  datatablePage: DatatablePageComponent,
  rrdPage: RrdPageComponent
};

type RouteConfig = {
  url: string;
  title?: string;
  editing?: boolean;
  notificationTitle?: string;
  component: {
    type:
      | 'navigationPage'
      | 'formPage'
      | 'selectionListPage'
      | 'textPage'
      | 'tabsPage'
      | 'datatablePage'
      | 'rrdPage';
    config: Record<string, any>;
  };
};

@Injectable({
  providedIn: 'root'
})
export class RouteConfigService {
  public configs$: Observable<Routes>;

  private configsSource = new ReplaySubject<Routes>(1);

  constructor(private http: HttpClient) {
    this.configs$ = this.configsSource.asObservable();
  }

  public load(): Observable<Routes> {
    return this.http.get('./assets/route-config.json').pipe(
      catchError((error) => {
        if (_.isFunction(error.preventDefault)) {
          error.preventDefault();
        }
        return of([]);
      }),
      map((configs: Array<RouteConfig>) => {
        const routes: Routes = [];
        // Convert the loaded route configuration into Angular
        // 'Route' objects.
        _.forEach(configs, (config) => {
          routes.push({
            path: config.url,
            component: componentMap[config.component.type],
            data: {
              title: config.title,
              editing: config.editing,
              notificationTitle: config.notificationTitle,
              config: config.component.config
            }
          });
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
      // Get the custom routes to inject at the given node.
      const filteredRoutes: Routes = _.filter(customRoutes, (customRoute: Route) => {
        const segments = _.split(_.trim(customRoute.path, '/'), '/');
        return segments[0] === rootSegment;
      });
      _.forEach(filteredRoutes, (filteredRoute: Route) => {
        // eslint-disable-next-line no-shadow
        const getParentNode = (routes: Routes, segments: Array<string>): Routes => {
          if (!segments.length) {
            return routes;
          }
          const segment: string = segments.shift();
          let node = _.find(routes, (route: Route) => route.path === segment);
          if (_.isUndefined(node)) {
            // Create the missing node.
            node = {
              path: segment,
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
                data: _.pick(node.data, ['config'])
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
        const segments: Array<string> = _.split(_.trim(filteredRoute.path, '/'), '/');
        // Remove the first segment.
        segments.shift();
        // Remove the last segment.
        let childPath: string = segments.pop();
        if (_.startsWith(childPath, ':')) {
          // Process paths like 'edit/:name'.
          childPath = [segments.pop(), childPath].join('/');
        }

        const parentNode = getParentNode(targetNode, segments);
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
