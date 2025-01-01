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
import { Component, OnDestroy, OnInit } from '@angular/core';
import { Subscription } from 'rxjs';

import {
  NavigationConfigService,
  NavigationMenuItem
} from '~/app/core/services/navigation-config.service';

@Component({
  selector: 'omv-navigation-bar',
  templateUrl: './navigation-bar.component.html',
  styleUrls: ['./navigation-bar.component.scss']
})
export class NavigationBarComponent implements OnInit, OnDestroy {
  public menuItems: NavigationMenuItem[];

  private subscription: Subscription;

  constructor(private navigationConfig: NavigationConfigService) {}

  ngOnInit(): void {
    this.subscription = this.navigationConfig.configs$.subscribe((menuItems) => {
      this.menuItems = menuItems;
    });
  }

  ngOnDestroy(): void {
    this.subscription.unsubscribe();
  }
}
