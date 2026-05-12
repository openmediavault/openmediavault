import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'nl2br'
})
export class Nl2brPipe implements PipeTransform {
  transform(value: any): any {
    if (!_.isString(value)) {
      return value;
    }
    return value.replace(/\r\n|\n/g, '<br>\n');
  }
}
