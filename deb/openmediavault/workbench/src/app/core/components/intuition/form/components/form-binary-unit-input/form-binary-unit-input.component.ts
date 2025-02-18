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

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { ClipboardService } from '~/app/shared/services/clipboard.service';

@Component({
  selector: 'omv-form-binary-unit-input',
  templateUrl: './form-binary-unit-input.component.html',
  styleUrls: ['./form-binary-unit-input.component.scss']
})
export class FormBinaryUnitInputComponent extends AbstractFormFieldComponent {
  constructor(private clipboardService: ClipboardService) {
    super();
  }

  onCopyToClipboard(): void {
    const control = this.formGroup.get(this.config.name);
    this.clipboardService.copy(control.value);
  }
}
