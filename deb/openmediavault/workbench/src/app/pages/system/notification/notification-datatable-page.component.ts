import { Component } from '@angular/core';

import { SelectionListPageConfig } from '~/app/core/components/limn-ui/models/selection-list-page-config.type';

@Component({
  template: '<omv-limn-selection-list-page [config]="this.config"></omv-limn-selection-list-page>'
})
export class NotificationDatatablePageComponent {
  public config: SelectionListPageConfig = {
    multiple: true,
    textProp: 'title',
    valueProp: 'uuid',
    selectedProp: 'enable',
    updateStoreOnSelectionChange: true,
    store: {
      proxy: {
        service: 'Notification',
        get: {
          method: 'getList'
        },
        post: {
          method: 'setList',
          filter: {
            mode: 'pick',
            props: ['uuid', 'id', 'enable']
          }
        }
      },
      sorters: [
        {
          dir: 'asc',
          prop: 'title'
        }
      ]
    },
    buttons: [
      {
        template: 'submit'
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/notification'
        }
      }
    ]
  };
}
