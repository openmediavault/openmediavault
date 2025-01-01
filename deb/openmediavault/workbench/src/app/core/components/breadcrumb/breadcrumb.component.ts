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
import { ChangeDetectionStrategy, ChangeDetectorRef, Component } from '@angular/core';
import {
  ActivatedRoute,
  ActivatedRouteSnapshot,
  Event,
  NavigationEnd,
  Router,
  UrlSegment
} from '@angular/router';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';
import { filter, startWith } from 'rxjs/operators';

import { Unsubscribe } from '~/app/decorators';
import { format, formatDeep } from '~/app/functions.helper';
import { Icon } from '~/app/shared/enum/icon.enum';
import { RpcService } from '~/app/shared/services/rpc.service';

export type Breadcrumb = {
  text: string;
  url: string;
  loading: boolean;
};

@Component({
  selector: 'omv-breadcrumb',
  templateUrl: './breadcrumb.component.html',
  styleUrls: ['./breadcrumb.component.scss'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class BreadcrumbComponent {
  @Unsubscribe()
  private subscriptions: Subscription = new Subscription();

  public breadcrumbs: Breadcrumb[] = [];
  public icon = Icon;

  constructor(
    private activatedRoute: ActivatedRoute,
    private cd: ChangeDetectorRef,
    private router: Router,
    private rpcService: RpcService
  ) {
    this.subscriptions.add(
      this.router.events
        .pipe(
          filter((event: Event) => event instanceof NavigationEnd),
          // The first 'NavigationEnd' event is already fired on page
          // load when this component is instantiated, so simply emit
          // a value before the router regularly begins to emit events
          // to render the breadcrumbs of the current activated route.
          startWith(true)
        )
        .subscribe(() => {
          const breadcrumbs = this.parseRoute(this.activatedRoute.snapshot.root);
          this.breadcrumbs = _.uniqWith(breadcrumbs, _.isEqual);
          this.cd.markForCheck();
        })
    );
  }

  private parseRoute(routeSnapshot: ActivatedRouteSnapshot): Breadcrumb[] {
    let routeParts: Breadcrumb[] = [];
    if (routeSnapshot.data.title || routeSnapshot.data.breadcrumb) {
      // Note, a `breadcrumb` configuration overrules `title`.
      let urlSegments: UrlSegment[] = [];
      routeSnapshot.pathFromRoot.forEach((routerState: ActivatedRouteSnapshot) => {
        urlSegments = urlSegments.concat(routerState.url);
      });
      const url: string = urlSegments
        .map((urlSegment) => encodeURIComponent(urlSegment.path))
        .join('/');
      const breadcrumbConfig = routeSnapshot.data?.breadcrumb;
      const isRequest: boolean = _.isPlainObject(breadcrumbConfig?.request);
      const formatData: Record<string, any> = {
        _routeParams: routeSnapshot.params,
        _routeQueryParams: routeSnapshot.queryParams
      };
      const routePart: Breadcrumb = {
        text: isRequest
          ? '...'
          : format(breadcrumbConfig?.text || routeSnapshot.data.title, formatData),
        url: '/' + url,
        loading: isRequest
      };
      if (isRequest) {
        const requestParams: Record<string, any> = formatDeep(
          _.get(breadcrumbConfig.request, 'params', {}),
          formatData
        );
        this.rpcService
          .request(breadcrumbConfig.request.service, breadcrumbConfig.request.method, requestParams)
          .subscribe((resp: Record<string, any>) => {
            routePart.text = format(
              breadcrumbConfig.text,
              _.merge(
                {
                  _routeParams: routeSnapshot.params
                },
                resp
              )
            );
            routePart.loading = false;
            this.cd.markForCheck();
          });
      }
      routeParts.push(routePart);
    }
    if (routeSnapshot.firstChild) {
      routeParts = routeParts.concat(this.parseRoute(routeSnapshot.firstChild));
    }
    return routeParts;
  }
}
