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
import { AfterViewInit, Directive, ElementRef, Input } from '@angular/core';
import * as _ from 'lodash';

@Directive({
  selector: '[autofocus]' // eslint-disable-line
})
export class AutofocusDirective implements AfterViewInit {
  private focus = true;

  constructor(private elementRef: ElementRef) {}

  @Input()
  set autofocus(condition: any) {
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

  ngAfterViewInit() {
    const el: HTMLInputElement = this.elementRef.nativeElement;
    if (this.focus && _.isFunction(el.focus)) {
      // Otherwise Angular throws error: Expression has changed after
      // it was checked.
      setTimeout(() => {
        el.focus();
      }, 100);
    }
  }
}
