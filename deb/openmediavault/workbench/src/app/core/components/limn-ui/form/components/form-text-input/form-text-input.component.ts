/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';
import { Icon } from '~/app/shared/enum/icon.enum';
import { ClipboardService } from '~/app/shared/services/clipboard.service';

@Component({
  selector: 'omv-form-text-input',
  templateUrl: './form-text-input.component.html',
  styleUrls: ['./form-text-input.component.scss']
})
export class FormTextInputComponent extends AbstractFormFieldComponent {
  public icon = Icon;

  constructor(private clipboardService: ClipboardService) {
    super();
  }

  onCopyToClipboard() {
    const control = this.formGroup.get(this.config.name);
    this.clipboardService.copy(control.value);
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    // Hide the 'Copy to clipboard' button if the browser does not
    // support that feature.
    if (!this.clipboardService.canCopy()) {
      this.config.hasCopyToClipboardButton = false;
    }
  }
}
