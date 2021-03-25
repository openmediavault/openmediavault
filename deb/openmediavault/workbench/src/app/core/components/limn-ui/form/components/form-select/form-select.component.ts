import { Component, OnInit } from '@angular/core';
import { MatSelectChange } from '@angular/material/select';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';
import { DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-form-select',
  templateUrl: './form-select.component.html',
  styleUrls: ['./form-select.component.scss']
})
export class FormSelectComponent extends AbstractFormFieldComponent implements OnInit {
  constructor(private dataStoreService: DataStoreService) {
    super();
  }

  ngOnInit(): void {
    super.ngOnInit();
    this.dataStoreService.load(this.config.store).subscribe(() => {
      if (this.config.hasEmptyOption) {
        const item = {};
        _.set(item, this.config.valueField, '');
        _.set(item, this.config.textField, this.config.emptyOptionText);
        this.config.store.data.unshift(item);
      }
    });

    const control = this.formGroup.get(this.config.name);
    control.valueChanges.subscribe((value) => {
      this.config.value = value;
    });
  }

  onSelectionChange(event: MatSelectChange) {
    if (_.isFunction(this.config.selectionChange)) {
      const value = _.clone(event.value);
      this.config.selectionChange(value);
    }
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      valueField: 'value',
      textField: 'text',
      hasEmptyOption: false,
      emptyOptionText: gettext('None'),
      store: {
        data: []
      }
    });
    if (_.isArray(this.config.store.data) && _.isUndefined(this.config.store.fields)) {
      _.merge(this.config.store, {
        fields: _.uniq([this.config.valueField, this.config.textField])
      });
    }
  }
}
