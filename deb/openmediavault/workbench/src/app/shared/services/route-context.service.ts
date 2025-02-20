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
import { Injectable } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  Router,
  RoutesRecognized,
  UrlSegment,
  UrlSegmentGroup
} from '@angular/router';
import _ from 'lodash';
import { filter } from 'rxjs/operators';

import { RouteContext } from '~/app/shared/models/route.model';

@Injectable({
  providedIn: 'root'
})
export class RouteContextService {
  private ctx: RouteContext;

  constructor(private router: Router) {
    this.reset();
    this.create();
    this.router.events
      .pipe(filter((event) => event instanceof RoutesRecognized))
      .subscribe((event: RoutesRecognized) => {
        this.reset();
        this.create(event.urlAfterRedirects, event.state.root);
      });
  }

  public get(): RouteContext {
    return this.ctx;
  }

  private create(newUrl?: string, snapshot?: ActivatedRouteSnapshot) {
    const urlTree = this.router.parseUrl(newUrl === undefined ? this.router.url : newUrl);
    this.ctx._routeUrlSegments = this.getUrlSegments(urlTree.root.children);

    const routeStack: ActivatedRouteSnapshot[] =
      snapshot === undefined ? [this.router.routerState.snapshot.root] : [snapshot];

    while (routeStack.length > 0) {
      const route = routeStack.pop();

      if (route.routeConfig !== null) {
        this.ctx._routeConfig.path = route.routeConfig.path;
        this.ctx._routeConfig.data = {
          ...this.ctx._routeConfig.data,
          ...route.data
        };
      }

      this.ctx._routeParams = { ...this.ctx._routeParams, ...route.params };
      this.ctx._routeQueryParams = {
        ...this.ctx._routeQueryParams,
        ...route.queryParams
      };

      routeStack.push(...route.children);
    }
  }

  private reset(): void {
    this.ctx = {
      _routeConfig: {
        path: '',
        data: {}
      },
      _routeParams: {},
      _routeQueryParams: {},
      _routeUrlSegments: []
    };
  }

  /**
   * @private
   */
  private getUrlSegments(children: { [key: string]: UrlSegmentGroup }): string[] {
    let segments: string[] = [];
    _.forEach(_.keys(children), (key: string) => {
      const urlSegmentGroup: UrlSegmentGroup = children[key];
      segments = segments
        .concat(urlSegmentGroup.segments.map((segment: UrlSegment) => segment.path))
        .concat(this.getUrlSegments(urlSegmentGroup.children));
    });
    return segments;
  }
}
