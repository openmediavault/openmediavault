import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import * as _ from 'lodash';

import {
  NavigationConfigService,
  NavigationMenuItem
} from '~/app/core/services/navigation-config.service';
import { Icon } from '~/app/shared/enum/icon.enum';

@Component({
  selector: 'omv-navigation-page',
  templateUrl: './navigation-page.component.html',
  styleUrls: ['./navigation-page.component.scss']
})
export class NavigationPageComponent implements OnInit {
  public menuItems: NavigationMenuItem[] = [];

  constructor(private router: Router, private navigationConfig: NavigationConfigService) {}

  ngOnInit(): void {
    this.navigationConfig
      .getChildrenByUrl(this.router.url)
      .subscribe((menuItems: NavigationMenuItem[]) => {
        this.menuItems = menuItems;
        this.sanitizeConfig();
      });
  }

  onClick(item: NavigationMenuItem) {
    if (item.url) {
      this.router.navigate([item.url]);
    }
  }

  protected sanitizeConfig() {
    _.forEach(this.menuItems, (menuItem: NavigationMenuItem) => {
      // Map icon from 'foo' to 'mdi:foo' if necessary.
      menuItem.icon = _.get(Icon, menuItem.icon, menuItem.icon);
    });
  }
}
