import { Injectable, OnDestroy } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { ActivatedRoute, Event, NavigationEnd, Router } from '@angular/router';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';
import { filter, switchMap } from 'rxjs/operators';

import { translate } from '~/app/i18n.helper';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';

const DEFAULT_TITLE = 'openmediavault Workbench';

@Injectable({
  providedIn: 'root'
})
export class TitleService implements OnDestroy {
  private subscription: Subscription;

  constructor(
    private router: Router,
    private systemInformationService: SystemInformationService,
    private title: Title
  ) {
    this.subscription = this.router.events
      .pipe(
        filter((event: Event) => event instanceof NavigationEnd),
        switchMap(() => {
          return this.systemInformationService.systemInfo$;
        })
      )
      .subscribe((res: SystemInformation) => {
        const titles: string[] = this.getTitles(this.router.routerState.root);
        const text: string = titles.join(' | ');
        const newTitle =
          (res.hostname.length ? `${res.hostname} - ` : '') +
          `${DEFAULT_TITLE}${titles.length ? ' - ' : ''}${text}`;
        // Use the following template for the page title:
        // (<HOSTNAME> - )?<DEFAULT_TITLE>( - <BREADCRUMBS>)?
        this.title.setTitle(newTitle);
      });
  }

  ngOnDestroy(): void {
    this.subscription.unsubscribe();
  }

  private getTitles(route: ActivatedRoute): string[] {
    const titles: string[] = [];
    while (route?.firstChild) {
      if (route.snapshot.data.title) {
        titles.push(route.snapshot.data.title);
      }
      route = route.firstChild;
    }
    if (route.snapshot.data.title) {
      titles.push(route.snapshot.data.title);
    }
    return _.uniqWith(titles, _.isEqual).map((title) => translate(title));
  }
}
