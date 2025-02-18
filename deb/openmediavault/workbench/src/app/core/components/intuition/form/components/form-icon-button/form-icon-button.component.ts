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
import { Component, ViewEncapsulation } from '@angular/core';

import { FormButtonComponent } from '~/app/core/components/intuition/form/components/form-button/form-button.component';

@Component({
  selector: 'omv-form-icon-button',
  templateUrl: './form-icon-button.component.html',
  styleUrls: ['./form-icon-button.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class FormIconButtonComponent extends FormButtonComponent {}
