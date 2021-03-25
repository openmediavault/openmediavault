import { AfterViewInit, Directive, ElementRef, Input } from '@angular/core';
import * as _ from 'lodash';

@Directive({
  selector: '[autofocus]' // eslint-disable-line
})
export class AutofocusDirective implements AfterViewInit {
  @Input()
  public set autofocus(condition: any) {
    this.focus =
      condition !== false &&
      condition !== 0 &&
      condition !== undefined &&
      condition !== null &&
      condition !== 'false' &&
      condition !== '0' &&
      condition !== 'undefined' &&
      condition !== 'null';
  }

  private focus = true;

  constructor(private elementRef: ElementRef) {}

  ngAfterViewInit() {
    const el: HTMLInputElement = this.elementRef.nativeElement;
    if (this.focus && _.isFunction(el.focus)) {
      // Otherwise Angular throws error: Expression has changed after
      // it was checked.
      setTimeout(() => {
        el.focus();
      });
    }
  }
}
