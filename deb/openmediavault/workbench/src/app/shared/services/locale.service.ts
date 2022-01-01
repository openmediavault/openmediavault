/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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

import { Locale } from '~/app/shared/enum/locale.enum';

@Injectable({
  providedIn: 'root'
})
export class LocaleService {
  /**
   * Get the browser locale.
   *
   * @return The browser locale, e.g. 'en_GB', 'de_DE'.
   */
  static getBrowserLocale(): string {
    let result: string;
    if (_.isArray(navigator.languages)) {
      result = _.chain<string>(navigator.languages)
        .filter((l: string) => l.includes('-'))
        .map((l: string) => l.replace('-', '_'))
        .filter((l: string) => _.has(Locale, l))
        .first()
        .value();
    }
    return _.defaultTo(result, 'en_GB');
  }

  static getLocale(): string {
    return localStorage.getItem('locale') || LocaleService.getBrowserLocale();
  }

  static setLocale(locale: string) {
    localStorage.setItem('locale', locale);
  }

  /**
   * Get a list of supported locales.
   *
   * @return A dictionary of locales, e.g. {
   *     'en_GB': 'English',
   *     'de_DE': 'Deutsch'
   *   }.
   */
  static getLocales(): Record<string, string> {
    return _.reduce(
      _.keys(Locale),
      (d, k) => {
        d[k] = _.get(Locale, k);
        return d;
      },
      {}
    );
  }
}
