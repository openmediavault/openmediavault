import { Directive, HostBinding, Input, OnChanges } from '@angular/core';

@Directive({
  selector: 'a[href]' // eslint-disable-line
})
export class ExternalLinkDirective implements OnChanges {
  @HostBinding('attr.href')
  hrefAttr = '';

  @HostBinding('attr.rel')
  relAttr = '';

  @HostBinding('attr.target')
  targetAttr = '';

  @Input()
  href: string;

  ngOnChanges(): void {
    this.hrefAttr = this.href;
    if (this.isExternal()) {
      this.relAttr = 'noopener';
      this.targetAttr = '_blank';
    }
  }

  private isExternal() {
    return !this.href.includes(location.hostname);
  }
}
