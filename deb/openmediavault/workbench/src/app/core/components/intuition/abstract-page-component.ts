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
import { ActivatedRoute, Router } from '@angular/router';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';

import { PageHintConfig } from '~/app/core/components/intuition/models/page.type';
import { formatDeep, isFormatable } from '~/app/functions.helper';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { PageContext, PageContextService } from '~/app/shared/services/pagecontext-service';

@Directive()
// eslint-disable-next-line @angular-eslint/directive-class-suffix
export abstract class AbstractPageComponent<T> implements AfterViewInit, OnInit, OnDestroy {
  @Input()
  config: T;

  // Event emitted when the form has been created.
  @Output()
  readonly afterViewInitEvent = new EventEmitter();

  pageContext: PageContext;

  private activatedRouteSubscription: Subscription;

  protected constructor(
    protected activatedRoute: ActivatedRoute,
    protected authSessionService: AuthSessionService,
    protected router: Router,
    protected pageContextService: PageContextService
  ) {
    this.pageContext = pageContextService.getContext();

    // Is the component configured via route data?
    if (_.has(this.pageContext._routeConfig, 'data.config')) {
      this.config = _.cloneDeep(_.get(this.pageContext._routeConfig, 'data.config')) as T;
    }
  }

  ngOnInit(): void {
    this.sanitizeConfig();
    this.onRouteParams();
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
   * @param props The list of tokenized properties to format.
   */
  protected formatConfig(props: Array<string>): void {
    _.forEach(props, (prop) => {
      const value = _.get(this.config as Record<string, any>, prop);
      if (isFormatable(value)) {
        _.set(this.config as Record<string, any>, prop, formatDeep(value, this.pageContext));
      }
    });
  }
}
