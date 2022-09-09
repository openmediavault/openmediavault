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
import { Pipe, PipeTransform } from '@angular/core';
import dayjs from 'dayjs';
import relativeTime from 'dayjs/plugin/relativeTime';
import * as _ from 'lodash';

dayjs.extend(relativeTime);

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
    if (!(_.isString(value) || _.isDate(value) || _.isNumber(value))) {
      return '';
    }
    let date: dayjs.Dayjs;
    if (_.isNumber(value)) {
      date = dayjs.unix(value);
    } else {
      date = dayjs(value);
    }
    if (!date.isValid()) {
      return '';
    }
    let result;
    switch (format) {
      case 'relative':
        result = date.fromNow(options);
        break;
      default:
        result = this.toLocaleString(date.toDate(), format);
        break;
    }
    return result;
  }

  toLocaleString(date: Date, format?: 'datetime' | 'time' | 'date'): string {
    let result;
    switch (format) {
      case 'datetime':
        result = date.toLocaleString();
        break;
      case 'time':
        result = date.toLocaleTimeString();
        break;
      case 'date':
        result = date.toLocaleDateString();
        break;
    }
    return result;
  }
}
