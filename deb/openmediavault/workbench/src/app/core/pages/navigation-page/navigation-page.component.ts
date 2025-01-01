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

  constructor(
    private router: Router,
    private navigationConfig: NavigationConfigService
  ) {}

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
