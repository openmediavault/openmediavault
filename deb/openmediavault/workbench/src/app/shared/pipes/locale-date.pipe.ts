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
   * @param dateFormat The format to use, e.g. 'date', 'time', 'datetime'
   *   or 'relative'. Defaults to 'date'.
   * @params options Additional options.
   * @return The time in the given format or an empty string on failure.
   */
  transform(value: Date | string | number, dateFormat?: string, options?: any): any {
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
    switch (dateFormat) {
      case 'relative':
        result = date.fromNow(options);
        break;
      case 'datetime':
        result = date.toDate().toLocaleString();
        break;
      case 'time':
        result = date.toDate().toLocaleTimeString();
        break;
      case 'date':
      default:
        result = date.toDate().toLocaleDateString();
        break;
    }
    return result;
  }
}
