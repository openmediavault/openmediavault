import { Injectable, OnDestroy } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { ActivatedRoute, Event, NavigationEnd, Router } from '@angular/router';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';
import { filter } from 'rxjs/operators';

const DEFAULT_TITLE = 'openmediavault Workbench';

@Injectable({
  providedIn: 'root'
})
export class TitleService implements OnDestroy {
  private subscription: Subscription;

  constructor(private router: Router, private title: Title) {
    this.subscription = this.router.events
      .pipe(filter((event: Event) => event instanceof NavigationEnd))
      .subscribe(() => {
        const titles: string[] = this.getTitles(this.router.routerState.root);
        const text: string = titles.join(' | ');
        const newTitle = `${DEFAULT_TITLE}${titles.length ? ' - ' : ''}${text}`;
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
    return _.uniqWith(titles, _.isEqual);
  }
}
