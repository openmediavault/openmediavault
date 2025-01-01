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
import { Component, Input, OnInit } from '@angular/core';
import { MatSelectChange } from '@angular/material/select';
import * as _ from 'lodash';

import { Icon } from '~/app/shared/enum/icon.enum';
import { Constraint } from '~/app/shared/models/constraint.type';
import { Datatable } from '~/app/shared/models/datatable.interface';
import {
  DatatableAction,
  DatatableActionEnabledConstraintsFn
} from '~/app/shared/models/datatable-action.type';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';
import { ConstraintService } from '~/app/shared/services/constraint.service';
import { DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-datatable-actions',
  templateUrl: './datatable-actions.component.html',
  styleUrls: ['./datatable-actions.component.scss']
})
export class DatatableActionsComponent implements OnInit {
  @Input()
  selection: DatatableSelection;

  @Input()
  actions: DatatableAction[];

  @Input()
  table: Datatable;

  constructor(private dataStoreService: DataStoreService) {}

  ngOnInit(): void {
    this.sanitizeConfig();
    _.forEach(this.actions, (action) => {
      if (action.type === 'select') {
        this.dataStoreService.load(action.store).subscribe();
      }
    });
  }

  isDisabled(action: DatatableAction): boolean {
    if (_.isPlainObject(action.enabledConstraints)) {
      const validators: Array<DatatableActionEnabledConstraintsFn> = [];
      if (_.isBoolean(action.enabledConstraints.hasData)) {
        validators.push((selection, table) =>
          action.enabledConstraints.hasData ? table.data?.length > 0 : !table.data?.length
        );
      }
      if (_.isNumber(action.enabledConstraints.minSelected)) {
        validators.push((selection) => selection.length >= action.enabledConstraints.minSelected);
      }
      if (_.isNumber(action.enabledConstraints.maxSelected)) {
        validators.push((selection) => selection.length <= action.enabledConstraints.maxSelected);
      }
      if (_.isArray(action.enabledConstraints.constraint)) {
        _.forEach(action.enabledConstraints.constraint, (constraint: Constraint) => {
          validators.push(
            (selection) =>
              ConstraintService.filter(selection.selected, constraint).length === selection.length
          );
        });
      }
      if (_.isFunction(action.enabledConstraints.callback)) {
        validators.push(action.enabledConstraints.callback);
      }
      const enabled = _.every(validators, (validator) => validator(this.selection, this.table));
      return !enabled;
    }
    return false;
  }

  onButtonClick(action: DatatableAction): void {
    if (_.isFunction(action.click)) {
      action.click(action, this.table);
    }
  }

  onSelectionChange(event: MatSelectChange, action: DatatableAction): void {
    if (_.isFunction(action.selectionChange)) {
      action.selectionChange(action, event.value, this.table);
    }
  }

  protected sanitizeConfig(): void {
    _.forEach(this.actions, (action) => {
      // Map icon from 'foo' to 'mdi:foo' if necessary.
      action.icon = _.get(Icon, action.icon, action.icon);
      if (action.type === 'select') {
        _.defaultsDeep(action, {
          valueField: 'value',
          textField: 'text',
          store: {
            data: []
          }
        });
      }
    });
  }
}
