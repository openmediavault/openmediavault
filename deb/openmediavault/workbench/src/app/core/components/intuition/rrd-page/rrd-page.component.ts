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
import { HttpErrorResponse } from '@angular/common/http';
import { Component, Inject, OnInit } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { EMPTY } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { AbstractPageComponent } from '~/app/core/components/intuition/abstract-page-component';
import {
  RrdPageConfig,
  RrdPageGraphConfig
} from '~/app/core/components/intuition/models/rrd-page-config.type';
import { format, formatDeep, unixTimeStamp } from '~/app/functions.helper';
import { Icon } from '~/app/shared/enum/icon.enum';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { DataStoreService } from '~/app/shared/services/data-store.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-intuition-rrd-page',
  templateUrl: './rrd-page.component.html',
  styleUrls: ['./rrd-page.component.scss']
})
export class RrdPageComponent extends AbstractPageComponent<RrdPageConfig> implements OnInit {
  public monitoringEnabled = true;
  public error: HttpErrorResponse;
  public icon = Icon;
  public loading = false;
  public time: number;
  public tabs: Array<{
    label: string;
    graphs: Array<RrdPageGraphConfig>;
  }> = [];

  public monitoringDisabledMessage: string = gettext(
    "System monitoring is disabled. To enable it, please go to the <a href='#/system/monitoring'>settings page</a>."
  );

  constructor(
    @Inject(ActivatedRoute) activatedRoute: ActivatedRoute,
    @Inject(AuthSessionService) authSessionService: AuthSessionService,
    @Inject(Router) router: Router,
    private blockUiService: BlockUiService,
    private dataStoreService: DataStoreService,
    private rpcService: RpcService
  ) {
    super(activatedRoute, authSessionService, router);
    // Check if monitoring is enabled.
    this.rpcService.request('PerfStats', 'get').subscribe((resp) => {
      this.monitoringEnabled = _.get(resp, 'enable', false);
    });
  }

  override ngOnInit(): void {
    super.ngOnInit();
    this.time = unixTimeStamp();
    if (this.config?.store) {
      this.loading = true;
      this.dataStoreService
        .load(this.config.store)
        .pipe(
          catchError((error) => {
            this.error = error;
            return EMPTY;
          }),
          finalize(() => {
            this.loading = false;
          })
        )
        .subscribe(() => {
          _.forEach(this.config.store.data, (item: Record<any, any>) => {
            const label = format(this.config.label, item);
            const graphs: RrdPageGraphConfig[] = _.map(
              this.config.graphs,
              (graph: RrdPageGraphConfig) => formatDeep(graph, item) as RrdPageGraphConfig
            );
            this.tabs.push({ label, graphs });
          });
        });
    }
  }

  onGenerate() {
    this.blockUiService.start(gettext('Please wait, the statistic graphs will be regenerated ...'));
    this.rpcService
      .requestTask('Rrd', 'generate')
      .pipe(
        finalize(() => {
          this.blockUiService.stop();
        })
      )
      .subscribe(() => {
        // Force redrawing the images.
        this.time = unixTimeStamp();
      });
  }
}
