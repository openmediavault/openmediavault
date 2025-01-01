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
import { Component, ElementRef, EventEmitter, Input, Output } from '@angular/core';
import { FormGroup } from '@angular/forms';
import * as _ from 'lodash';

@Component({
  selector: 'omv-submit-button',
  templateUrl: './submit-button.component.html',
  styleUrls: ['./submit-button.component.scss']
})
export class SubmitButtonComponent {
  // Optional ID of the form this button is assigned to.
  @Input()
  form?: string;

  @Input()
  formGroup: FormGroup;

  @Input()
  type = 'submit';

  @Input()
  disabled = false;

  @Output()
  readonly buttonClick = new EventEmitter<Event>();

  constructor(private elementRef: ElementRef) {}

  onSubmit(event: Event) {
    // Set focus to the 'Submit' button.
    this.elementRef.nativeElement.focus();
    // Check if the form is invalid. If that is the case, then do
    // the following:
    // - Mark all invalid form fields as touched, thus they are
    //   automatically drawn in red and the error message is
    //   displayed below the form field.
    // - Focus the first invalid form field.
    if (this.formGroup && this.formGroup.invalid) {
      const invalidControls = _.filter(
        _.values(this.formGroup.controls),
        (control) => control.invalid
      );
      _.invokeMap(invalidControls, 'markAllAsTouched');
      _.invokeMap(invalidControls, 'updateValueAndValidity');
      const element = this.elementRef.nativeElement.offsetParent.querySelector(
        '.mat-input-element.ng-invalid'
      );
      if (element) {
        if (_.isFunction(element.focus)) {
          element.focus();
        }
        element.scrollIntoView({ behavior: 'smooth' });
      }
      return;
    }
    this.buttonClick.emit(event);
  }
}
