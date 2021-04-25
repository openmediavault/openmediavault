/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { HttpClient } from '@angular/common/http';
import { TranslateService } from '@ngx-translate/core';
import { TranslateLoader } from '@ngx-translate/core';
import * as _ from 'lodash';
import { Observable, of } from 'rxjs';
import { catchError } from 'rxjs/operators';

let translateService: TranslateService;

/**
 * Load the specified translation file.
 */
export class TranslateHttpLoader implements TranslateLoader {
  constructor(private http: HttpClient) {}

  public getTranslation(lang: string): Observable<Record<string, string>> {
    return this.http.get<Record<string, string>>(`/assets/i18n/${lang}.json`).pipe(
      catchError((error) => {
        // Do not show an error notification and return an empty language
        // dictionary in case of the translation file does not exist.
        if (_.isFunction(error.preventDefault)) {
          error.preventDefault();
        }
        return of({});
      })
    );
  }
}

/**
 * Set translation service globally for the `translate` helper function.
 *
 * @param service The translation service instance.
 */
export const setTranslationService = (service: TranslateService) => {
  translateService = service;
};

/**
 * Returns a translation instantly.
 *
 * @param key The string to be translated.
 * @return The translated string or if translation service is not
 *   available the untranslated string.
 */
export const translate = (key: string): string =>
  _.isUndefined(translateService) ? key : translateService.instant(key);
