import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'defaultTo'
})
export class DefaultToPipe implements PipeTransform {
  transform(value: any, defaultValue?: any): any {
    return _.isUndefined(value) || _.isNull(value) || _.isNaN(value) || value === ''
      ? defaultValue
      : value;
  }
}
