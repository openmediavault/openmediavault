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
import { TranslocoService } from '@ngneat/transloco';
import * as _ from 'lodash';

let translocoService: TranslocoService;

/**
 * The list of supported locales.
 */
export enum SupportedLocales {
  /* eslint-disable @typescript-eslint/naming-convention */
  'ar_SA' = 'العربية', // Arabic
  'bg_BG' = '​Български', // Bulgarian
  'ca_ES' = 'Catalán', // Catalan
  'cs_CZ' = 'Čeština', // Czech
  'da_DA' = 'Dansk', // Danish
  'de_DE' = 'Deutsch', // German
  'el_GR' = 'ελληνικά', // Greek
  'en_GB' = 'English', // English
  'es_ES' = 'Español', // Spanish
  'fr_FR' = 'Français', // French
  'gl_ES' = 'Galego', // Galician
  'hu_HU' = 'Magyar', // Hungarian
  'it_IT' = 'Italiano', // Italian
  'ja_JP' = '日本語', // Japanese (Japan)
  'ko_KR' = '한국어', // Korean
  'nl_NL' = 'Nederlands', // Dutch
  'no_NO' = 'Norsk', // Norwegian
  'pl_PL' = 'Polski', // Polish
  'pt_PT' = 'Português', // Portuguese
  'ru_RU' = 'Русский', // Russian
  'sl_SI' = 'Slovenski', // Slovenian
  'sv_SV' = 'Svenska', // Swedish
  'tr_TR' = 'Türkçe', // Turkish
  'uk_UK' = 'Українська', // Ukrainian
  'zh_CN' = '简体中文', // Chinese (Simplified Chinese)
  'zh_TW' = '繁體中文' // Chinese (Taiwan)
  /* eslint-enable @typescript-eslint/naming-convention */
}

/**
 * Get the browser locale. Defaults to `en_GB` if no locale is detected.
 *
 * @return The browser locale, e.g. `en_GB` or `de_DE`.
 */
const getBrowserLocale = (): string => {
  let result: string;
  if (_.isArray(navigator.languages)) {
    result = _.chain<string>(navigator.languages)
      .filter((l: string) => l.includes('-'))
      .map((l: string) => l.replace('-', '_'))
      .filter((l: string) => _.has(SupportedLocales, l))
      .first()
      .value();
  }
  return _.defaultTo(result, 'en_GB');
};

/**
 * Get the current locale. Try to get the setting from the local
 * storage of the browser, otherwise try to get it from the users
 * preferred languages in the browser.
 */
export const getCurrentLocale = (): string => {
  return localStorage.getItem('locale') || getBrowserLocale();
};

/**
 * Set the current locale. The setting will be stored in local storage
 * of the browser.
 */
export const setCurrentLocale = (locale: string): void => {
  localStorage.setItem('locale', locale);
};

/**
 * Set translation service globally for the `translate` helper function.
 *
 * @param service The translation service instance.
 */
export const setTranslationService = (service: TranslocoService) => {
  translocoService = service;
};

/**
 * Returns a translation instantly.
 *
 * @param key The string to be translated.
 * @return The translated string or if translation service is not
 *   available the untranslated string.
 */
export const translate = (key: string): string =>
  _.isUndefined(translocoService) ? key : translocoService.translate(key);
