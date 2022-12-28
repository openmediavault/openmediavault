import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'split'
})
export class SplitPipe implements PipeTransform {
  /**
   * Divides a string into an ordered list of substrings.
   *
   * @param value The value to process.
   * @param separator The pattern describing where each split should
   *   occur.
   * @param limit A non-negative integer specifying a limit on the
   *   number of substrings to be included in the array.
   */
  transform(value: unknown, separator?: string, limit?: number): string | string[] | unknown {
    if (!_.isString(value)) {
      return value;
    }
    return _.isEmpty(value) ? [] : value.split(separator, limit);
  }
}
