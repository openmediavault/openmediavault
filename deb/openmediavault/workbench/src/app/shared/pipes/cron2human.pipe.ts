import { Pipe, PipeTransform } from '@angular/core';

import { cron2human } from '~/app/functions.helper';

@Pipe({
  name: 'cron2human'
})
export class Cron2humanPipe implements PipeTransform {
  transform(value: string): string {
    return cron2human(value);
  }
}
