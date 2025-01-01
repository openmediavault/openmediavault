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
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { Icon } from '~/app/shared/enum/icon.enum';

@Component({
  selector: 'omv-form-datatable',
  templateUrl: './form-datatable.component.html',
  styleUrls: ['./form-datatable.component.scss']
})
export class FormDatatableComponent extends AbstractFormFieldComponent {
  protected override sanitizeConfig(): void {
    super.sanitizeConfig();
    if (this.config.hasSearchField || this.config.actions.length) {
      this.config.hasActionBar = true;
    }
    _.forEach(this.config.actions, (action) => {
      switch (action.template) {
        case 'add':
          _.defaultsDeep(action, {
            id: 'add',
            type: 'iconButton',
            tooltip: gettext('Add'),
            icon: Icon.add
          });
          break;
        case 'edit':
          _.defaultsDeep(action, {
            id: 'edit',
            type: 'iconButton',
            tooltip: gettext('Edit'),
            icon: Icon.edit,
            enabledConstraints: {
              minSelected: 1,
              maxSelected: 1
            }
          });
          break;
        case 'delete':
          _.defaultsDeep(action, {
            id: 'delete',
            type: 'iconButton',
            tooltip: gettext('Delete'),
            icon: Icon.delete,
            enabledConstraints: {
              minSelected: 1
            }
          });
          break;
        default:
          _.defaultsDeep(action, {
            id: 'custom',
            type: 'iconButton'
          });
          break;
      }
    });
  }
}
