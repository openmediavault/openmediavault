import { Component, OnInit, ViewChild } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { DatatablePageComponent } from '~/app/core/components/limn-ui/datatable-page/datatable-page.component';
import { DatatablePageConfig } from '~/app/core/components/limn-ui/models/datatable-page-config.type';
import { ILogConfig, LogConfigService } from '~/app/core/services/log-config.service';
import { translate } from '~/app/i18n.helper';
import { DatatableActionConfig } from '~/app/shared/models/datatable-action-config.type';
import { RpcService } from '~/app/shared/services/rpc.service';

@Component({
  template: '<omv-limn-datatable-page #page [config]="this.config"></omv-limn-datatable-page>'
})
export class SystemLogsListPageComponent implements OnInit {
  @ViewChild('page', { static: true })
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
        icon: 'mdi:download',
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

  constructor(private rpcService: RpcService, private logConfigService: LogConfigService) {}

  ngOnInit(): void {
    this.logConfigService.configs$.subscribe((logConfigs: Array<ILogConfig>) => {
      this.config.actions[0].store.data = _.chain(logConfigs)
        .sortBy(['text'])
        .map((systemLogConfig) => ({
          text: translate(systemLogConfig.text),
          value: systemLogConfig.id
        }))
        .value();
    });
  }

  onSelectionChange(action: DatatableActionConfig, value: string) {
    this.logConfigService.configs$.subscribe((logs: Array<ILogConfig>) => {
      const logConfig = logs.find((lc: ILogConfig) => lc.id === value);
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
        // Finally reload the datatable content. Note, we need to wait
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
