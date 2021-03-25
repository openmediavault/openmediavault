import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'map'
})
export class MapPipe implements PipeTransform {
  transform(value: string | number | boolean, map?: Record<any, any>): any {
    if (!_.isPlainObject(map)) {
      return value;
    }
    // @ts-ignore
    return _.get(map, value, value);
  }
}
