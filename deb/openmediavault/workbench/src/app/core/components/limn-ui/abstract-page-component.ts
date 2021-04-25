/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { ActivatedRoute, ParamMap, Route } from '@angular/router';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';

import { decodeURIComponentDeep, formatDeep, isFormatable } from '~/app/functions.helper';
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

  readonly routeConfig: Route;
  routeParams: Record<string, any> = {};
  private activatedRouteSubscription: Subscription;

  protected constructor(
    protected activatedRoute: ActivatedRoute,
    protected authSessionService: AuthSessionService
  ) {
    this.routeConfig = this.activatedRoute.routeConfig;
    // Is the component configured via route data?
    if (_.has(this.routeConfig, 'data.config')) {
      this.config = _.get(this.routeConfig, 'data.config');
    }
  }

  ngOnInit(): void {
    this.sanitizeConfig();
    this.activatedRouteSubscription = this.activatedRoute.params.subscribe((params: ParamMap) => {
      this.routeParams = decodeURIComponentDeep(params);
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
  protected sanitizeConfig() {}

  /**
   * A callback method that is invoked immediately after the observable
   * of the matrix parameters scoped to this route have been resolved.
   */
  protected onRouteParams() {}

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
      _routeParams: this.routeParams
    };
  }

  /**
   * Format the given configuration properties.
   *
   * @param paths The paths of the properties to format.
   */
  protected formatConfig(paths: Array<string>) {
    _.forEach(paths, (path) => {
      const value = _.get(this.config as Record<string, any>, path);
      if (isFormatable(value)) {
        _.set(this.config as Record<string, any>, path, formatDeep(value, this.pageContext));
      }
    });
  }
}
