import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
  name: 'max'
})
export class MaxPipe implements PipeTransform {
  transform(value: number, maxValue: number): number {
    return Math.max(value, maxValue);
  }
}
