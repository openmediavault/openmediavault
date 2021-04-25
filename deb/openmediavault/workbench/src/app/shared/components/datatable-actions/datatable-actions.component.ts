import { Component, Input, OnInit } from '@angular/core';
import { MatSelectChange } from '@angular/material/select';
import * as _ from 'lodash';

import { Icon } from '~/app/shared/enum/icon.enum';
import { Constraint } from '~/app/shared/models/constraint.type';
import { DatatableActionConfig } from '~/app/shared/models/datatable-action-config.type';
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
  actions: DatatableActionConfig[];

  // Optional data that will be used as argument for the action
  // `click` and `selectionChange` callback functions.
  @Input()
  data: any[];

  constructor(private dataStoreService: DataStoreService) {}

  ngOnInit(): void {
    this.sanitizeConfig();
    _.forEach(this.actions, (action) => {
      if (action.type === 'select') {
        this.dataStoreService.load(action.store).subscribe();
      }
    });
  }

  isDisabled(action: DatatableActionConfig) {
    if (_.isPlainObject(action.enabledConstraints)) {
      const validators = [];
      if (_.isBoolean(action.enabledConstraints.hasData)) {
        validators.push((selected, data) =>
          action.enabledConstraints.hasData ? data?.length : !data?.length
        );
      }
      if (_.isNumber(action.enabledConstraints.minSelected)) {
        validators.push((selected) => selected.length >= action.enabledConstraints.minSelected);
      }
      if (_.isNumber(action.enabledConstraints.maxSelected)) {
        validators.push((selected) => selected.length <= action.enabledConstraints.maxSelected);
      }
      if (_.isArray(action.enabledConstraints.constraint)) {
        _.forEach(action.enabledConstraints.constraint, (custom: Constraint) => {
          validators.push(
            (selected) => ConstraintService.filter(selected, custom).length === selected.length
          );
        });
      }
      if (_.isFunction(action.enabledConstraints.callback)) {
        validators.push(action.enabledConstraints.callback);
      }
      const enabled = _.every(validators, (validator) =>
        validator(this.selection.selected, this.data)
      );
      return !enabled;
    }
    return false;
  }

  onButtonClick(action: DatatableActionConfig) {
    if (_.isFunction(action.click)) {
      action.click(action, this.selection, this.data);
    }
  }

  onSelectionChange(event: MatSelectChange, action: DatatableActionConfig) {
    if (_.isFunction(action.selectionChange)) {
      action.selectionChange(action, event.value, this.data);
    }
  }

  protected sanitizeConfig() {
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
