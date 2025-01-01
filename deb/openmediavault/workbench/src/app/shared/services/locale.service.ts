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
import { Injectable } from '@angular/core';
import * as _ from 'lodash';

import { getCurrentLocale, setCurrentLocale, SupportedLocales } from '~/app/i18n.helper';

@Injectable({
  providedIn: 'root'
})
export class LocaleService {
  /**
   * Get the current locale.
   */
  static getCurrentLocale(): string {
    return getCurrentLocale();
  }

  /**
   * Set the current locale.
   */
  static setCurrentLocale(locale: string): void {
    setCurrentLocale(locale);
  }

  /**
   * Get an object of the supported locales, where the key is the country
   * code and the value is the long description of the locale.
   *
   * @return A dictionary of locales, e.g. {
   *     'en_GB': 'English',
   *     'de_DE': 'Deutsch'
   *   }.
   */
  static getSupportedLocales(): Record<string, string> {
    return _.reduce(
      _.keys(SupportedLocales),
      (d, k) => {
        d[k] = _.get(SupportedLocales, k);
        return d;
      },
      {}
    );
  }
}
