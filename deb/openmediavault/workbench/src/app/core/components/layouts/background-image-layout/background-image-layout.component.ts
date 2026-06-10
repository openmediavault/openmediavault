/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
import { ActivatedRoute, NavigationEnd, Router } from '@angular/router';
import { Subscription } from 'rxjs';
import { filter } from 'rxjs/operators';

import { Unsubscribe } from '~/app/decorators';

@Component({
  selector: 'omv-background-image-layout',
  templateUrl: './background-image-layout.component.html',
  styleUrls: ['./background-image-layout.component.scss']
})
export class BackgroundImageLayoutComponent implements OnInit {
  @Unsubscribe()
  private subscriptions = new Subscription();

  public backgroundImage: string;

  constructor(
    private activatedRoute: ActivatedRoute,
    private router: Router
  ) {
    this.backgroundImage = "url('./assets/images/login.jpg')";
  }

  ngOnInit(): void {
    // Update background on initial load.
    this.updateBackgroundImage();

    // Update background on every navigation to a child route.
    this.subscriptions.add(
      this.router.events.pipe(filter((event) => event instanceof NavigationEnd)).subscribe(() => {
        this.updateBackgroundImage();
      })
    );
  }

  private updateBackgroundImage(): void {
    const route = this.findDeepestChild(this.activatedRoute);
    if (route) {
      const data = route.snapshot.data;
      const backgroundImage: string = data?.backgroundImage ?? 'login.jpg';
      this.backgroundImage = `url('./assets/images/${backgroundImage}')`;
    }
  }

  private findDeepestChild(route: ActivatedRoute): ActivatedRoute | null {
    let child = route.firstChild;
    while (child?.firstChild) {
      child = child.firstChild;
    }
    return child;
  }
}
