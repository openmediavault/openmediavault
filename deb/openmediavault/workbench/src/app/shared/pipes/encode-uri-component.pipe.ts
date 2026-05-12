import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'encodeUriComponent'
})
export class EncodeUriComponentPipe implements PipeTransform {
  transform(value: any): any {
    if (!_.isString(value) && !_.isNumber(value) && !_.isBoolean(value)) {
      return value;
    }
    return encodeURIComponent(value);
  }
}
