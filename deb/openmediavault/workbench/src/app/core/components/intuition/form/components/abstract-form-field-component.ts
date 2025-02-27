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
import { Directive, inject, Input, OnInit } from '@angular/core';
import { FormGroup } from '@angular/forms';
import * as _ from 'lodash';

import { formatFormFieldConfig } from '~/app/core/components/intuition/functions.helper';
import { FormFieldConfig } from '~/app/core/components/intuition/models/form-field-config.type';
import { PageContext } from '~/app/core/models/page-context.type';
import { PageContextService } from '~/app/core/services/page-context.service';
import { Icon } from '~/app/shared/enum/icon.enum';

@Directive()
// eslint-disable-next-line @angular-eslint/directive-class-suffix
export abstract class AbstractFormFieldComponent implements OnInit {
  @Input()
  config: FormFieldConfig;

  @Input()
  formGroup: FormGroup;

  public icon = Icon;

  protected pageContext: PageContext = inject(PageContextService).get();

  ngOnInit(): void {
    this.sanitizeConfig();
    this.formatConfig();
  }

  /**
   * Sanitize the configuration, e.g. set default values or convert
   * properties.
   * @protected
   */
  protected sanitizeConfig(): void {
    // Map icon from 'foo' to 'mdi:foo' if necessary.
    this.config.icon = _.get(Icon, this.config.icon, this.config.icon);
  }

  /**
   * Format several tokenized form field properties.
   * @protected
   */
  protected formatConfig(): void {
    formatFormFieldConfig([this.config], this.pageContext, ['value']);
  }
}
