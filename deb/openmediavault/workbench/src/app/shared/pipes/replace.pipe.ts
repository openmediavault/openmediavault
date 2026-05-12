import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'replace'
})
export class ReplacePipe implements PipeTransform {
  transform(value: any, searchValue: string | RegExp, replaceValue: string): any {
    if (!_.isString(value) || _.isUndefined(searchValue) || _.isUndefined(replaceValue)) {
      return value;
    }
    return value.replace(searchValue, replaceValue);
  }
}
