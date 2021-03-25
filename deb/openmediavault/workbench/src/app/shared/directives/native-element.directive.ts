import { Directive, ElementRef, OnInit } from '@angular/core';
import { NgControl } from '@angular/forms';
import * as _ from 'lodash';

@Directive({
  // eslint-disable-next-line @angular-eslint/directive-selector
  selector: '[formControlName]'
})
export class NativeElementDirective implements OnInit {
  constructor(private elementRef: ElementRef, private control: NgControl) {}

  ngOnInit(): void {
    _.set(this.control.control, 'nativeElement', this.elementRef.nativeElement);
  }
}
