import { HttpErrorResponse } from '@angular/common/http';
import { Component, Inject, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { BlockUI, NgBlockUI } from 'ng-block-ui';
import { EMPTY } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { AbstractPageComponent } from '~/app/core/components/limn-ui/abstract-page-component';
import {
  RrdPageConfig,
  RrdPageGraphConfig
} from '~/app/core/components/limn-ui/models/rrd-page-config.type';
import { format, formatDeep, unixTimeStamp } from '~/app/functions.helper';
import { Icon } from '~/app/shared/enum/icon.enum';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { DataStoreService } from '~/app/shared/services/data-store.service';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  selector: 'omv-limn-rrd-page',
  templateUrl: './rrd-page.component.html',
  styleUrls: ['./rrd-page.component.scss']
})
export class RrdPageComponent extends AbstractPageComponent<RrdPageConfig> implements OnInit {
  @BlockUI()
  blockUI: NgBlockUI;

  public error: HttpErrorResponse;
  public icon = Icon;
  public loading = false;
  public time: number;
  public tabs: Array<{
    label: string;
    graphs: Array<RrdPageGraphConfig>;
  }> = [];

  constructor(
    @Inject(ActivatedRoute) activatedRoute: ActivatedRoute,
    @Inject(AuthSessionService) authSessionService: AuthSessionService,
    private dataStoreService: DataStoreService,
    private rpcService: RpcService
  ) {
    super(activatedRoute, authSessionService);
  }

  ngOnInit(): void {
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
    this.blockUI.start(gettext('Please wait, the statistic graphs will be regenerated ...'));
    this.rpcService
      .requestTask('Rrd', 'generate')
      .pipe(
        finalize(() => {
          this.blockUI.stop();
        })
      )
      .subscribe(() => {
        // Force redrawing the images.
        this.time = unixTimeStamp();
      });
  }
}
