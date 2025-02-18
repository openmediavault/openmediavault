import { inject, Injectable } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  Params,
  Router,
  RoutesRecognized,
  UrlSegment,
  UrlSegmentGroup
} from '@angular/router';
import _ from 'lodash';
import { filter } from 'rxjs/operators';

import { Permissions } from '~/app/shared/models/permissions.model';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';

export type PageContext = {
  _session: {
    username: string;
    permissions: Permissions;
  };
  _routeConfig: {
    path: string;
    data: object;
  };
  _routeParams: Params;
  _routeQueryParams: Params;
  _routeUrlSegments: string[];
  _editing: boolean;
  _selected: object | undefined;
};

@Injectable({
  providedIn: 'root'
})
export class PageContextService {
  private pageContext: PageContext;

  private authSessionService: AuthSessionService = inject(AuthSessionService);
  private router: Router = inject(Router);

  constructor() {
    this.resetPageContext();
    this.fillPageContext();

    this.router.events
      .pipe(filter((event) => event instanceof RoutesRecognized))
      .subscribe((event: RoutesRecognized) => {
        this.resetPageContext();
        this.fillPageContext(event.urlAfterRedirects, event.state.root);
      });
  }

  private fillPageContext(newUrl?: string, snapshot?: ActivatedRouteSnapshot) {
    const urlTree = this.router.parseUrl(newUrl === undefined ? this.router.url : newUrl);
    this.pageContext._routeUrlSegments = this.getUrlSegments(urlTree.root.children);

    const routeStack: ActivatedRouteSnapshot[] =
      snapshot === undefined ? [this.router.routerState.snapshot.root] : [snapshot];

    while (routeStack.length > 0) {
      const route = routeStack.pop();

      if (route.routeConfig !== null) {
        this.pageContext._routeConfig.path = route.routeConfig.path;
        this.pageContext._routeConfig.data = {
          ...this.pageContext._routeConfig.data,
          ...route.data
        };
      }

      this.pageContext._routeParams = { ...this.pageContext._routeParams, ...route.params };
      this.pageContext._routeQueryParams = {
        ...this.pageContext._routeQueryParams,
        ...route.queryParams
      };

      routeStack.push(...route.children);
    }

    // @ts-ignore
    this.pageContext._editing = this.pageContext._routeConfig.data.editing ?? false;
  }

  private resetPageContext(): void {
    this.pageContext = {
      _session: {
        username: '',
        permissions: {}
      },
      _routeConfig: {
        path: '',
        data: {}
      },
      _routeParams: {},
      _routeQueryParams: {},
      _routeUrlSegments: [],
      _editing: false,
      _selected: undefined
    };
  }

  // eslint-disable-next-line @typescript-eslint/member-ordering
  public getContext(): PageContext {
    const contextWithUser = this.pageContext;

    contextWithUser._session.username = this.authSessionService.getUsername();
    contextWithUser._session.permissions = this.authSessionService.getPermissions();

    return contextWithUser;
  }

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
