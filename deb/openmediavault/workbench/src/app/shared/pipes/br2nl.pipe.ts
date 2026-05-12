import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'br2nl'
})
export class Br2nlPipe implements PipeTransform {
  transform(value: any): any {
    if (!_.isString(value)) {
      return value;
    }
    return value.replace(/<br[ \t]*\/?>/g, '\n');
  }
}
