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
/* eslint-disable @typescript-eslint/member-ordering */
import { coerceBooleanProperty } from '@angular/cdk/coercion';
import { Component, Input, Optional, Self, ViewEncapsulation } from '@angular/core';
import { ControlValueAccessor, NgControl } from '@angular/forms';
import { MatFormFieldControl } from '@angular/material/form-field';
import * as _ from 'lodash';
import { Subject } from 'rxjs';

let nextUniqueId = 0;

@Component({
  // eslint-disable-next-line @angular-eslint/component-selector
  selector: 'mat-form-button',
  exportAs: 'matFormButton',
  templateUrl: './mat-form-button.component.html',
  styleUrls: ['./mat-form-button.component.scss'],
  providers: [
    {
      provide: MatFormFieldControl,
      useExisting: MatFormButtonComponent
    }
  ],
  encapsulation: ViewEncapsulation.None
})
export class MatFormButtonComponent implements ControlValueAccessor, MatFormFieldControl<any> {
  /**
   * Implemented as part of MatFormFieldControl.
   */
  public value: any; // Useless for a button.
  public required: boolean; // Useless for a button.
  public placeholder: string; // Useless for a button.
  public empty = false; // Useless for a button.
  public stateChanges = new Subject<void>();
  public focused = false;
  public shouldLabelFloat = false;
  public errorState = false;
  public controlType = 'button';

  private _disabled = false;
  private _uniqueId = `mat-form-button-${++nextUniqueId}`;

  constructor(@Optional() @Self() public ngControl: NgControl) {
    if (!_.isNull(this.ngControl)) {
      this.ngControl.valueAccessor = this;
    }
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  @Input()
  get disabled(): boolean {
    return this._disabled;
  }
  set disabled(value: boolean) {
    this._disabled = coerceBooleanProperty(value);
    this.stateChanges.next();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  get id(): string {
    return this._uniqueId;
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  setDescribedByIds(ids: string[]): void {
    // Nothing to do here.
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  onContainerClick(event: MouseEvent): void {
    // Nothing to do here.
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  writeValue(value: any): void {
    // Nothing to do here.
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  registerOnChange(fn: (value: any) => void): void {
    // Nothing to do here.
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  registerOnTouched(fn: any): void {
    // Nothing to do here.
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  setDisabledState(isDisabled: boolean): void {
    this.disabled = isDisabled;
  }
}
