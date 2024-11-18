import { Directive, ElementRef, HostListener, Renderer2 } from '@angular/core';

/**
 * A directive that adds the classes 'loading', 'loaded' or 'error'
 * to the image element depending on the loading state.
 */
@Directive({
  // eslint-disable-next-line @angular-eslint/directive-selector
  selector: 'img[loadingState]'
})
export class LoadingStateDirective {
  constructor(
    private elementRef: ElementRef,
    private renderer: Renderer2
  ) {}

  @HostListener('loadstart') onLoadStart() {
    this.renderer.addClass(this.elementRef.nativeElement, 'loading');
  }

  @HostListener('load') onLoad() {
    this.renderer.removeClass(this.elementRef.nativeElement, 'loading');
    this.renderer.addClass(this.elementRef.nativeElement, 'loaded');
  }

  @HostListener('error') onError() {
    this.renderer.removeClass(this.elementRef.nativeElement, 'loading');
    this.renderer.addClass(this.elementRef.nativeElement, 'error');
  }
}
