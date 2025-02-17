import { Injectable } from '@angular/core';
import { ActivatedRoute, Params, Router, UrlSegment, UrlSegmentGroup } from '@angular/router';
import * as _ from 'lodash';

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

@Injectable()
export class PageContextService {
  selection: object | undefined;

  constructor(
    private authSessionService: AuthSessionService,
    private activatedRoute: ActivatedRoute,
    private router: Router
  ) {}

  public getContext(): PageContext {
    const urlTree = this.router.parseUrl(this.router.url);
    const urlSegments = this.getUrlSegments(urlTree.root.children);

    const config = {
      path: this.activatedRoute.routeConfig?.path ?? '',
      data: this.activatedRoute.routeConfig?.data ?? {}
    };

    let currentParams: Params;
    this.activatedRoute.params.subscribe((params) => {
      currentParams = params;
    });

    let currentQueryParams: Params;
    this.activatedRoute.queryParams.subscribe((queryParams) => {
      currentQueryParams = queryParams;
    });

    return {
      _session: {
        username: this.authSessionService.getUsername(),
        permissions: this.authSessionService.getPermissions()
      },
      _routeConfig: config,
      _routeParams: currentParams,
      _routeQueryParams: currentQueryParams,
      _routeUrlSegments: urlSegments,
      _editing: config.data.editing ?? false,
      _selected: undefined
    };
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
