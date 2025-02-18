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
import { AbstractControl } from '@angular/forms';
import * as _ from 'lodash';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';

@Component({
  selector: 'omv-form-file-input',
  templateUrl: './form-file-input.component.html',
  styleUrls: ['./form-file-input.component.scss']
})
export class FormFileInputComponent extends AbstractFormFieldComponent {
  onChange(event: Event) {
    const file: File = (event.target as HTMLInputElement).files[0];
    const reader = new FileReader();
    reader.addEventListener('load', this.onFileReaderLoad.bind(this));
    reader.readAsText(file, 'utf8');
  }

  protected onFileReaderLoad(event: ProgressEvent<FileReader>) {
    const control: AbstractControl = this.formGroup.get(this.config.name);
    const trim = _.defaultTo(this.config.trim, this.config.rows === 1);
    control.setValue(trim ? event.target.result.toString().trim() : event.target.result);
    control.markAsTouched();
    control.markAsDirty();
    control.updateValueAndValidity();
  }
}
