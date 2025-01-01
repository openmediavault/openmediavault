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
import { Component } from '@angular/core';

import { SelectionListPageConfig } from '~/app/core/components/intuition/models/selection-list-page-config.type';
import { BaseSelectionListPageComponent } from '~/app/pages/base-page-component';

@Component({
  template:
    '<omv-intuition-selection-list-page [config]="this.config"></omv-intuition-selection-list-page>'
})
export class NotificationSelectionListPageComponent extends BaseSelectionListPageComponent {
  public config: SelectionListPageConfig = {
    hasSelectAllButton: true,
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
