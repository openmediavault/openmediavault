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
import { AbstractControl } from '@angular/forms';
import * as _ from 'lodash';

import { AbstractFormFieldComponent } from '~/app/core/components/limn-ui/form/components/abstract-form-field-component';

@Component({
  selector: 'omv-form-file-input',
  templateUrl: './form-file-input.component.html',
  styleUrls: ['./form-file-input.component.scss']
})
export class FormFileInputComponent extends AbstractFormFieldComponent {
  fileUpload(files: FileList, controlName: string) {
    const file: File = files[0];
    const reader = new FileReader();
    reader.addEventListener('load', (event: ProgressEvent<FileReader>) => {
      const control: AbstractControl = this.formGroup.get(controlName);
      control.setValue(
        this.config.rows > 1 ? event.target.result : event.target.result.toString().trim()
      );
      control.markAsTouched();
      control.markAsDirty();
      control.updateValueAndValidity();
    });
    reader.readAsText(file, 'utf8');
  }

  protected sanitizeConfig() {
    super.sanitizeConfig();
    _.defaultsDeep(this.config, {
      rows: 4
    });
  }
}
