import { Directive, ElementRef, HostListener, Input, OnInit } from '@angular/core';
import { NgControl } from '@angular/forms';
import * as _ from 'lodash';

import { binaryUnit, toBytes } from '~/app/functions.helper';

@Directive({
  // eslint-disable-next-line @angular-eslint/directive-selector
  selector: '[binaryUnit]'
})
export class BinaryUnitDirective implements OnInit {
  // The default unit used when the value does not contain a unit.
  // Defaults to bytes (`B`).
  @Input()
  defaultUnit?: 'B' | 'KiB' | 'MiB' | 'GiB' | 'TiB' | 'PiB' | 'EiB' | 'ZiB' | 'YiB';

  // The number of digits after the decimal point.
  // Defaults to `0`.
  @Input()
  fractionDigits?: number;

  private el: HTMLInputElement;

  constructor(
    private elementRef: ElementRef,
    private ngControl: NgControl
  ) {
    this.el = this.elementRef.nativeElement;
  }

  @HostListener('blur', ['$event.target.value'])
  onBlur(value: string) {
    this.setValue(value);
  }

  ngOnInit() {
    this.defaultUnit = _.defaultTo(this.defaultUnit, 'B');
    this.fractionDigits = _.defaultTo(this.fractionDigits, 0);
    this.setValue(this.el.value);
  }

  setValue(value: string) {
    if (/^[\d.]+$/.test(value)) {
      value += this.defaultUnit;
    }
    // Reformat input to preferred appearance.
    const bytes = toBytes(value);
    value = binaryUnit(bytes, this.fractionDigits);
    if (!_.isEmpty(value)) {
      this.el.value = value;
      this.ngControl.control.setValue(value);
    }
  }
}
