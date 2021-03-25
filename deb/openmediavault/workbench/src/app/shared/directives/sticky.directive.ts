import { Directive, ElementRef, Input, OnDestroy, OnInit } from '@angular/core';
import * as _ from 'lodash';
import { animationFrameScheduler, fromEvent, Subscription } from 'rxjs';
import { filter, takeWhile, throttleTime } from 'rxjs/operators';

@Directive({
  // eslint-disable-next-line @angular-eslint/directive-selector
  selector: '[sticky]'
})
export class StickyDirective implements OnInit, OnDestroy {
  @Input()
  scrollContainer?: string;

  @Input()
  disabled? = false;

  @Input()
  stickyClass = 'omv-sticky';

  private subscriptions = new Subscription();

  constructor(private elementRef: ElementRef) {}

  ngOnInit(): void {
    this.subscriptions.add(
      fromEvent(window, 'scroll', { capture: true, passive: false, once: false })
        .pipe(
          takeWhile(() => !this.disabled),
          filter(() => {
            // Get the position of the element relative to the viewport.
            const rect = this.elementRef.nativeElement.getBoundingClientRect();
            // Process only valid position information. It may happen that
            // 'getBoundingClientRect' returns position information with
            // all properties set to 0. This may happen when this element
            // is not rendered properly or is not in the DOM at the time
            // the event occurs. Seems to happen for Angular Material tab
            // components embedded within perfect scrollbar elements.
            return _.every(_.values(rect.toJSON()), (value) => value !== 0);
          }),
          throttleTime(0, animationFrameScheduler)
        )
        .subscribe(() => {
          const elementRect = this.elementRef.nativeElement.getBoundingClientRect();
          const scrollContainerElement = document.querySelector(this.scrollContainer);
          const scrollContainerOffsetTop = (scrollContainerElement as HTMLElement).offsetTop;
          this.setSticky(scrollContainerOffsetTop >= elementRect.top);
        })
    );
  }

  ngOnDestroy(): void {
    this.subscriptions.unsubscribe();
  }

  private setSticky(isSticky: boolean): void {
    const classList = this.elementRef.nativeElement.classList;
    if (isSticky) {
      classList.add(this.stickyClass);
    } else {
      classList.remove(this.stickyClass);
    }
  }
}
