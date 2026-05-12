import { Directive, ElementRef, Input } from '@angular/core';

@Directive({
  selector: '[autocapitalize]' // eslint-disable-line
})
export class AutocapitalizeDirective {
  constructor(private elementRef: ElementRef) {}

  @Input()
  set autocapitalize(value: 'off' | 'none' | 'on' | 'sentences' | 'words' | 'characters') {
    this.elementRef.nativeElement.setAttribute('autocapitalize', value);
  }
}
