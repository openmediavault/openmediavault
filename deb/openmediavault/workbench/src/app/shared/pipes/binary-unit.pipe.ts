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
