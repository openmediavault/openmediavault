/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
import { AfterViewInit, Directive, EventEmitter, Input, OnInit, Output } from '@angular/core';
import * as _ from 'lodash';
import { EMPTY, Observable } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { PageHintConfig } from '~/app/core/components/intuition/models/page-config.type';
import { PageContext } from '~/app/core/models/page-context.type';
import { PageContextService } from '~/app/core/services/page-context.service';
import { formatDeep, isFormatable } from '~/app/functions.helper';
import { RpcObjectResponse } from '~/app/shared/models/rpc.model';

@Directive()
// eslint-disable-next-line @angular-eslint/directive-class-suffix
export abstract class AbstractPageComponent<T> implements AfterViewInit, OnInit {
  @Input()
  config: T;

  // Event emitted when the form has been created.
  @Output()
  readonly afterViewInitEvent = new EventEmitter();

  protected constructor(public pageContextService: PageContextService) {
    // Is the component configured via route data?
    if (_.has(this.pageContext._routeConfig, 'data.config')) {
      this.config = _.cloneDeep(_.get(this.pageContext._routeConfig, 'data.config')) as T;
    }
  }

  /**
   * Helper method to get the page related configuration that can be
   * used in tokenized strings. It contains the route configuration and
   * parameters. Note, to do not collide with form field values, the
   * property names start with underscores.
   */
  get pageContext(): PageContext {
    return this.pageContextService.get();
  }

  ngOnInit(): void {
    this.sanitizeConfig();
    this.onPageInit();
  }

  ngAfterViewInit(): void {
    this.afterViewInitEvent.emit();
  }

  loadData(...args: any): Observable<RpcObjectResponse> {
    this.pageContextService.startLoading();
    return this.doLoadData(...args).pipe(
      catchError((error: any) => {
        this.pageContextService.setError(error);
        return EMPTY;
      }),
      finalize(() => {
        this.pageContextService.stopLoading();
      })
    );
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
  protected onPageInit(): void {}

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

  /**
   * Load the data for this page. Override this method to load the data.
   * @returns An observable that emits the loaded data.
   */
  protected doLoadData(...args: any): Observable<any> {
    return EMPTY;
  }

  /**
   * Use this function body as a template for processing the data loaded using
   * `doLoadData`. Override this method in your code.
   * @param res The response data.
   */
  protected onLoadData(res: any): void {}
}
