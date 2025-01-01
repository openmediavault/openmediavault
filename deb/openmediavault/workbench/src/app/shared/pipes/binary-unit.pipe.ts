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

import { binaryUnit } from '~/app/functions.helper';

/**
 * Convert a number of bytes into the highest possible binary unit.
 * Example: 2097152 will become 2.00 MiB
 */
@Pipe({
  name: 'binaryUnit'
})
export class BinaryUnitPipe implements PipeTransform {
  /**
   * @param value The number of bytes to be converted.
   * @param fractionDigits The number of digits after the decimal point.
   *   Defaults to 2.
   * @param maxUnit The max. unit to be used, e.g. 'KiB' or 'MiB'.
   * @return The converted value, e.g. '2.50 MiB'. Negative or empty
   *   input values will return an empty string.
   */
  transform(value: number | string, fractionDigits?: number, maxUnit?: string): string {
    if (_.isString(value) && _.isEmpty(value)) {
      return '';
    }
    return binaryUnit(_.toNumber(value), fractionDigits, maxUnit);
  }
}
