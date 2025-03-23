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
import { Component, OnInit, ViewChild } from '@angular/core';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';

import { DatatablePageComponent } from '~/app/core/components/intuition/datatable-page/datatable-page.component';
import { DatatablePageConfig } from '~/app/core/components/intuition/models/datatable-page-config.type';
import { LogConfig, LogConfigService } from '~/app/core/services/log-config.service';
import { translate } from '~/app/i18n.helper';
import { DatatableAction } from '~/app/shared/models/datatable-action.type';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  template: '<omv-intuition-datatable-page [config]="this.config"></omv-intuition-datatable-page>'
})
export class SystemLogsListPageComponent implements OnInit {
  @ViewChild(DatatablePageComponent, { static: true })
  page: DatatablePageComponent;

  public config: DatatablePageConfig = {
    autoLoad: false,
    autoReload: false,
    remoteSorting: true,
    remotePaging: true,
    limit: 15,
    selectionType: 'none',
    sorters: [],
    columns: [],
    store: {
      data: []
    },
    actions: [
      {
        type: 'select',
        placeholder: gettext('Select a system log ...'),
        selectionChange: this.onSelectionChange.bind(this),
        store: {
          data: []
        }
      },
      {
        type: 'divider'
      },
      {
        type: 'iconButton',
        icon: 'download',
        tooltip: gettext('Download'),
        click: this.onDownload.bind(this),
        enabledConstraints: {
          callback: () =>
            // Enable the download button if a log ID is selected.
            _.isString(this.logId)
        }
      }
    ]
  };

  private logId: string;

  constructor(
    private rpcService: RpcService,
    private logConfigService: LogConfigService
  ) {}

  ngOnInit(): void {
    this.logConfigService.configs$.subscribe((logConfigs: Array<LogConfig>) => {
      this.config.actions[0].store.data = _.chain(logConfigs)
        .sortBy(['text'])
        .map((systemLogConfig) => ({
          text: translate(systemLogConfig.text),
          value: systemLogConfig.id
        }))
        .value();
    });
  }

  onSelectionChange(action: DatatableAction, value: string) {
    this.logConfigService.configs$.subscribe((logs: Array<LogConfig>) => {
      const logConfig = logs.find((lc: LogConfig) => lc.id === value);
      if (!_.isUndefined(logConfig)) {
        this.logId = value;
        // Update the configuration of the datatable.
        // Do not use _.merge(), otherwise Angular CD will not detect
        // the changes.
        this.config.stateId = logConfig.id;
        this.config.columns = logConfig.columns;
        this.config.sorters = logConfig.sorters;
        this.config.store.proxy = {
          service: logConfig.request.service,
          get: {
            method: logConfig.request.method,
            params: logConfig.request.params
          }
        };
        // Finally, reload the datatable content. Note, we need to wait
        // until the datatable has updated the configuration (e.g. sorters)
        // thanks to Angular's change-detection.
        setTimeout(() => {
          this.page.table.offset = 0;
          this.page.table.updateColumns();
          this.page.table.reloadData();
        });
      }
    });
  }

  onDownload() {
    this.rpcService.download('LogFile', 'getContent', { id: this.logId });
  }
}
