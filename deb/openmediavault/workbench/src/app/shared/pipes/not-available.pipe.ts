import { Pipe, PipeTransform } from '@angular/core';

import { notAvailable } from '~/app/functions.helper';

@Pipe({
  name: 'notAvailable'
})
export class NotAvailablePipe implements PipeTransform {
  transform(value: any, text?: string): any {
    return notAvailable(value, text);
  }
}
