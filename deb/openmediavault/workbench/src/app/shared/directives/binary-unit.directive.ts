import { Directive, ElementRef, HostListener, Input, OnInit } from '@angular/core';
import { NgControl } from '@angular/forms';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';

import { Unsubscribe } from '~/app/decorators';
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

  @Unsubscribe()
  private subscriptions = new Subscription();

  private readonly el: HTMLInputElement;

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
    this.setValue(this.ngControl.control.value);
    this.subscriptions.add(
      this.ngControl.control.valueChanges.subscribe((value) => {
        // Do not reformat while the user is actively typing into the
        // field, otherwise it would be impossible to enter a value at
        // all, e.g. because the cursor jumps to the end after every
        // keystroke. The `blur` handler takes care of that case.
        if (document.activeElement !== this.el) {
          this.setValue(value);
        }
      })
    );
  }

  setValue(value: string | number) {
    if (/^[\d.]+$/.test(String(value))) {
      value = `${value}${this.defaultUnit}`;
    }
    // Reformat input to preferred appearance.
    const bytes = toBytes(value);
    const formatted = binaryUnit(bytes, this.fractionDigits);
    // Only update the control if the formatted value actually changed to
    // avoid triggering an endless `valueChanges` loop.
    if (!_.isEmpty(formatted) && formatted !== this.ngControl.control.value) {
      this.el.value = formatted;
      this.ngControl.control.setValue(formatted);
    }
  }
}
