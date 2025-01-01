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
import { BooleanInput, coerceBooleanProperty } from '@angular/cdk/coercion';
import { COMMA, ENTER } from '@angular/cdk/keycodes';
import { Component, HostBinding, Input, Optional, Self } from '@angular/core';
import { ControlValueAccessor, NgControl } from '@angular/forms';
import { MatChipInputEvent } from '@angular/material/chips';
import { MatFormFieldControl } from '@angular/material/form-field';
import * as _ from 'lodash';
import { Subject } from 'rxjs';

import { Icon } from '~/app/shared/enum/icon.enum';

let nextUniqueId = 0;

@Component({
  // eslint-disable-next-line @angular-eslint/component-selector
  selector: 'mat-form-tag-input',
  exportAs: 'matFormTagInput',
  templateUrl: './mat-form-tag-input.component.html',
  styleUrls: ['./mat-form-tag-input.component.scss'],
  providers: [
    {
      provide: MatFormFieldControl,
      useExisting: MatFormTagInputComponent
    }
  ]
})
export class MatFormTagInputComponent implements ControlValueAccessor, MatFormFieldControl<string> {
  // eslint-disable-next-line @typescript-eslint/naming-convention
  static ngAcceptInputType_required: BooleanInput;

  /**
   * Implemented as part of MatFormFieldControl.
   */
  public stateChanges = new Subject<void>();
  public controlType = 'tag-input';

  public icon = Icon;
  public readonly separatorKeysCodes = [ENTER, COMMA] as const;

  private _value: string[] = [];
  private _focused = false;
  private _disabled = false;
  private _required = false;
  private _placeholder: string;
  private _uniqueId = `mat-form-tag-input-${++nextUniqueId}`;

  // @ts-ignore
  private onChange = (_value: any) => {};
  // @ts-ignore
  private onTouched = () => {};

  constructor(@Optional() @Self() public ngControl: NgControl) {
    if (!_.isNull(this.ngControl)) {
      this.ngControl.valueAccessor = this;
    }
  }

  @Input()
  separator?: string = String.fromCharCode(COMMA);

  @Input()
  get value(): string {
    return this._value.join(this.separator);
  }
  set value(value: string) {
    const tags: string[] = _.isEmpty(value) ? [] : value.split(this.separator);
    if (!_.isEqual(tags, this._value)) {
      this._value = tags;
      this.onChange(value);
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
  @Input()
  get placeholder(): string {
    return this._placeholder;
  }
  set placeholder(value: string) {
    this._placeholder = value;
    this.stateChanges.next();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  @Input()
  get required(): boolean {
    return this._required;
  }
  set required(value: boolean) {
    this._required = coerceBooleanProperty(value);
    this.stateChanges.next();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  get focused(): boolean {
    return this._focused;
  }
  set focused(value: boolean) {
    this._focused = coerceBooleanProperty(value);
    this.stateChanges.next();
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  get errorState(): boolean {
    return this.ngControl?.touched && this.ngControl?.invalid;
  }

  /**
   * Implemented as part of MatFormFieldControl.
   */
  get empty(): boolean {
    return !this._value.length;
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
  writeValue(value: string): void {
    this.value = value;
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  registerOnChange(fn: (value: any) => void): void {
    this.onChange = fn;
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  registerOnTouched(fn: any): void {
    this.onTouched = fn;
  }

  /**
   * Implemented as part of ControlValueAccessor.
   */
  setDisabledState(disabled: boolean): void {
    this.disabled = disabled;
  }

  @HostBinding('class.floating')
  get shouldLabelFloat() {
    return this.focused || !this.empty;
  }

  onAdd(event: MatChipInputEvent): void {
    const tag = (event.value || '').trim();
    if (tag) {
      this._value.push(tag);
      this._value = _.uniq(this._value).sort();
      this.onChange(this.value);
    }
    event.chipInput?.clear();
  }

  onRemove(tag: string): void {
    const index = this._value.indexOf(tag);
    if (index >= 0) {
      this._value.splice(index, 1);
      this.onChange(this.value);
    }
  }

  onBlur(event: Event): void {
    this.onTouched();
    this.stateChanges.next();
  }
}
