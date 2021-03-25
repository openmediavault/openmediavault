import { Pipe, PipeTransform } from '@angular/core';

import { toBoolean } from '~/app/functions.helper';

@Pipe({
  name: 'toBoolean'
})
export class ToBooleanPipe implements PipeTransform {
  transform(value: any): boolean {
    return toBoolean(value);
  }
}
