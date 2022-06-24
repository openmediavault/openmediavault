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
import { TranslocoService } from '@ngneat/transloco';
import * as _ from 'lodash';

let translocoService: TranslocoService;

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
