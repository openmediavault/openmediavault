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
import * as _ from 'lodash';

/**
 * Truncates string if it's longer than the given maximum string length.
 */
@Pipe({
  name: 'truncate'
})
export class TruncatePipe implements PipeTransform {
  /**
   * @param value The string to truncate.
   * @param length The maximum string length.
   * @param omission The string to indicate text is omitted.
   *   Defaults to '...'.
   * @param separator The separator pattern to truncate to.
   * @return Returns the truncated string.
   */
  transform(value: string, length: number, omission?: string, separator?: RegExp | string): string {
    if (!(_.isString(value) || _.isDate(value))) {
      return value;
    }
    omission = _.defaultTo(omission, '...');
    return _.truncate(value, { length, omission, separator });
  }
}
