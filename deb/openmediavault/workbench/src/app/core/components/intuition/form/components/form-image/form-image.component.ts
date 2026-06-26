/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
import { Component, ViewEncapsulation } from '@angular/core';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';

/**
 * Displays a read-only image whose source URL is taken from the form control
 * value (i.e. the field with `name` in the form group). Typically used to show
 * a QR code or other data-URL image inside a declarative form page without
 * requiring any user input.
 *
 * Set `submitValue: false` in the field config to exclude it from the
 * submitted form values.
 */
@Component({
  selector: 'omv-form-image',
  templateUrl: './form-image.component.html',
  styleUrls: ['./form-image.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class FormImageComponent extends AbstractFormFieldComponent {}
