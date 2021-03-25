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
