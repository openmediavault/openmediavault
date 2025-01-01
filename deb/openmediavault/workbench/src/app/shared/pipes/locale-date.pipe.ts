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
import { Pipe, PipeTransform } from '@angular/core';

import { toLocaleDate } from '~/app/functions.helper';

@Pipe({
  name: 'localeDate',
  pure: false
})
export class LocaleDatePipe implements PipeTransform {
  /**
   * Convert the specified date/time appropriate to the host environment's
   * current locale or into relative time like '2 hours ago'.
   *
   * @param value The date/time value. If it is a number, a UNIX epoch
   *   timestamp is assumed.
   * @param format The format to use, e.g. 'date', 'time', 'datetime'
   *   or 'relative'. Defaults to 'date'.
   * @params options Additional options.
   * @return The time in the given format or an empty string on failure.
   */
  transform(
    value: Date | string | number,
    format?: 'relative' | 'datetime' | 'time' | 'date',
    options?: any
  ): any {
    return toLocaleDate(value, format, options);
  }
}
