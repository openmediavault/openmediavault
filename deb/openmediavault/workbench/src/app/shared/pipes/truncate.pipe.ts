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
