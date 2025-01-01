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
import { Component, Input, OnInit } from '@angular/core';
import { Event, NavigationEnd, NavigationStart, Router } from '@angular/router';
import * as _ from 'lodash';
import { Subscription } from 'rxjs';
import { filter } from 'rxjs/operators';

import { NavigationBarListItem } from '~/app/core/components/navigation-bar/navigation-bar-list-item/navigation-bar-list-item.type';
import { Unsubscribe } from '~/app/decorators';
import { Icon } from '~/app/shared/enum/icon.enum';

@Component({
  selector: 'omv-navigation-bar-list-item',
  templateUrl: './navigation-bar-list-item.component.html',
  styleUrls: ['./navigation-bar-list-item.component.scss']
})
export class NavigationBarListItemComponent implements OnInit {
  @Input()
  item: NavigationBarListItem;

  @Input()
  depth = 0;

  @Unsubscribe()
  private subscriptions = new Subscription();

  public icon = Icon;

  constructor(private router: Router) {
    this.subscriptions.add(
      this.router.events
        .pipe(filter((event: Event) => event instanceof NavigationStart))
        .subscribe((event: NavigationStart) => {
          this.item.active = false;
          this.item.expanded = _.startsWith(event.url, this.item.url);
        })
    );
    this.subscriptions.add(
      this.router.events
        .pipe(filter((event: Event) => event instanceof NavigationEnd))
        .subscribe((event: NavigationEnd) => {
          if (this.item.children && this.item.children.length > 0) {
            // If the item has children, the compare the whole URL.
            this.item.active = event.url === this.item.url;
          } else {
            // If the item is a leaf, then compare the beginning of the
            // string, so the item will be displayed as active if a page
            // is shown that is using parts of the items URL, e.g.:
            // - Route of the menu item: /system/powermgmt/tasks/
            // - Route of the displayed page: /system/powermgmt/tasks/create
            this.item.active = _.startsWith(event.url, this.item.url);
          }
        })
    );
  }

  ngOnInit(): void {
    if (this.item.children && this.item.children.length > 0) {
      this.item.active = this.router.url === this.item.url;
    } else {
      this.item.active = _.startsWith(this.router.url, this.item.url);
    }
    this.item.expanded = _.startsWith(this.router.url, this.item.url);
  }

  onClick(item: NavigationBarListItem) {
    if (item.children && item.children.length > 0) {
      item.expanded = !item.expanded;
    }
    if (item.url) {
      this.router.navigate([item.url]);
    }
  }
}
