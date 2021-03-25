import { Directive, Input, OnInit } from '@angular/core';
import { FormGroup } from '@angular/forms';
import * as _ from 'lodash';

import { FormFieldConfig } from '~/app/core/components/limn-ui/models/form-field-config.type';
import { Icon } from '~/app/shared/enum/icon.enum';

@Directive()
// eslint-disable-next-line @angular-eslint/directive-class-suffix
export abstract class AbstractFormFieldComponent implements OnInit {
  @Input()
  config: FormFieldConfig;

  @Input()
  formGroup: FormGroup;

  ngOnInit(): void {
    this.sanitizeConfig();
  }

  /**
   * Sanitize the configuration, e.g. set default values or convert
   * properties.
   */
  protected sanitizeConfig() {
    // Map icon from 'foo' to 'mdi:foo' if necessary.
    this.config.icon = _.get(Icon, this.config.icon, this.config.icon);
  }
}
