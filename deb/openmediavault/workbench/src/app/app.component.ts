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
import { Component, Renderer2 } from '@angular/core';

import {
  PrefersColorScheme,
  PrefersColorSchemeService
} from '~/app/shared/services/prefers-color-scheme.service';

@Component({
  selector: 'omv-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent {
  constructor(
    private prefersColorSchemeService: PrefersColorSchemeService,
    private renderer2: Renderer2
  ) {
    this.prefersColorSchemeService.change$.subscribe(
      (prefersColorScheme: PrefersColorScheme): void => {
        if (prefersColorScheme === 'dark') {
          this.renderer2.addClass(document.body, 'omv-dark-theme');
        } else {
          this.renderer2.removeClass(document.body, 'omv-dark-theme');
        }
      }
    );
  }
}
