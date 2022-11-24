/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import { ChangeDetectionStrategy, ChangeDetectorRef, Component, OnDestroy } from '@angular/core';
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

import { Icon } from '~/app/shared/enum/icon.enum';

export type Breadcrumb = {
  text: string;
  url: string;
};

@Component({
  selector: 'omv-breadcrumb',
  templateUrl: './breadcrumb.component.html',
  styleUrls: ['./breadcrumb.component.scss'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class BreadcrumbComponent implements OnDestroy {
  public breadcrumbs: Breadcrumb[] = [];
  public icon = Icon;

  private subscription: Subscription;

  constructor(
    private activatedRoute: ActivatedRoute,
    private cd: ChangeDetectorRef,
    private router: Router
  ) {
    this.subscription = this.router.events
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
      });
  }

  ngOnDestroy(): void {
    this.subscription.unsubscribe();
  }

  private parseRoute(routeSnapshot: ActivatedRouteSnapshot): Breadcrumb[] {
    let routeParts: Breadcrumb[] = [];
    if (routeSnapshot.data.title) {
      let urlSegments: UrlSegment[] = [];
      routeSnapshot.pathFromRoot.forEach((routerState: ActivatedRouteSnapshot) => {
        urlSegments = urlSegments.concat(routerState.url);
      });
      const url = urlSegments.map((urlSegment) => urlSegment.path).join('/');
      routeParts.push({
        text: routeSnapshot.data.title,
        url: '/' + url
      });
    }
    if (routeSnapshot.firstChild) {
      routeParts = routeParts.concat(this.parseRoute(routeSnapshot.firstChild));
    }
    return routeParts;
  }
}
