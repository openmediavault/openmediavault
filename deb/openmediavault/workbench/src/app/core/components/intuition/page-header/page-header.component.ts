import { Component, Input } from '@angular/core';
import { Subscription } from 'rxjs';

import { PageContextService, PageStatus } from '~/app/core/services/page-context.service';
import { CoerceBoolean, Unsubscribe } from '~/app/decorators';

@Component({
  selector: 'omv-intuition-page-header',
  templateUrl: './page-header.component.html',
  styleUrls: ['./page-header.component.scss']
})
export class PageHeaderComponent {
  @CoerceBoolean()
  @Input()
  showLoadingMessage? = true;

  @Unsubscribe()
  private subscriptions = new Subscription();

  protected status: PageStatus;

  constructor(protected pageContextService: PageContextService) {
    this.subscriptions.add(
      this.pageContextService.status$.subscribe((status: PageStatus) => {
        this.status = status;
      })
    );
  }
}
