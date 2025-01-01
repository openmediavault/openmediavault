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
import {
  AfterViewInit,
  Directive,
  EventEmitter,
  Input,
  OnDestroy,
  OnInit,
  Output
} from '@angular/core';
import {
  ActivatedRoute,
  Params,
  Route,
  Router,
  UrlSegment,
  UrlSegmentGroup
} from '@angular/router';
import * as _ from 'lodash';
import { combineLatest, Subscription } from 'rxjs';

import { PageHintConfig } from '~/app/core/components/intuition/models/page-config.type';
import { decodeURIComponentDeep, format, formatDeep, isFormatable } from '~/app/functions.helper';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';

export type PageContext = Record<string, any>;

@Directive()
// eslint-disable-next-line @angular-eslint/directive-class-suffix
export abstract class AbstractPageComponent<T> implements AfterViewInit, OnInit, OnDestroy {
  @Input()
  config: T;

  // Event emitted when the form has been created.
  @Output()
  readonly afterViewInitEvent = new EventEmitter();

  readonly routeUrlSegments: string[];
  readonly routeConfig: Route;
  routeParams: Params = {};
  routeQueryParams: Params = {};

  private activatedRouteSubscription: Subscription;

  protected constructor(
    protected activatedRoute: ActivatedRoute,
    protected authSessionService: AuthSessionService,
    protected router: Router
  ) {
    this.routeConfig = this.activatedRoute.routeConfig;

    const urlTree = this.router.parseUrl(this.router.url);
    this.routeUrlSegments = this.getUrlSegments(urlTree.root.children);

    // Is the component configured via route data?
    if (_.has(this.routeConfig, 'data.config')) {
      this.config = _.cloneDeep(_.get(this.routeConfig, 'data.config')) as T;
    }
  }

  /**
   * Helper method to get the page related configuration that can be
   * used in tokenized strings. It contains the route configuration and
   * parameters. Note, to do not collide with form field values, the
   * property names start with underscores.
   */
  get pageContext(): PageContext {
    return {
      _session: {
        username: this.authSessionService.getUsername(),
        permissions: this.authSessionService.getPermissions()
      },
      _routeConfig: this.routeConfig,
      _routeParams: this.routeParams,
      _routeQueryParams: this.routeQueryParams,
      _routeUrlSegments: this.routeUrlSegments
    };
  }

  ngOnInit(): void {
    this.sanitizeConfig();
    this.activatedRouteSubscription = combineLatest([
      this.activatedRoute.params,
      this.activatedRoute.queryParams
    ]).subscribe(([params, queryParams]: Params[]) => {
      this.routeParams = decodeURIComponentDeep(params);
      this.routeQueryParams = decodeURIComponentDeep(queryParams);
      this.onRouteParams();
    });
  }

  ngOnDestroy(): void {
    this.activatedRouteSubscription?.unsubscribe();
  }

  ngAfterViewInit(): void {
    this.afterViewInitEvent.emit();
  }

  /**
   * Sanitize the configuration, e.g. set default values or convert
   * properties.
   */
  protected sanitizeConfig(): void {}

  /**
   * Sanitize the hint configuration.
   */
  protected sanitizeHintsConfig(): void {
    const hints: PageHintConfig[] = _.get(this.config, 'hints', []) as PageHintConfig[];
    _.forEach(hints, (hintConfig: PageHintConfig) => {
      _.defaultsDeep(hintConfig, {
        type: 'info',
        dismissible: false
      });
    });
  }

  /**
   * A callback method that is invoked immediately after the observable
   * of the matrix parameters scoped to this route have been resolved.
   */
  protected onRouteParams(): void {}

  /**
   * Format the given configuration properties using the page context.
   *
   * @param paths The paths of the properties to format.
   */
  protected formatConfig(paths: Array<string>): void {
    _.forEach(paths, (path) => {
      const value = _.get(this.config as Record<string, any>, path);
      if (isFormatable(value)) {
        _.set(this.config as Record<string, any>, path, formatDeep(value, this.pageContext));
      }
    });
  }

  /**
   * Format the hint configuration using the page context.
   *
   * @protected
   */
  protected formatHintsConfig(): void {
    const hints: PageHintConfig[] = _.get(this.config, 'hints', []) as PageHintConfig[];
    _.forEach(hints, (hintConfig: PageHintConfig) => {
      if (isFormatable(hintConfig.text)) {
        hintConfig.text = format(hintConfig.text, this.pageContext);
      }
    });
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
