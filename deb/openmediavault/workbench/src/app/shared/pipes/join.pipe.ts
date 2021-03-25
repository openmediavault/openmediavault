import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
  name: 'join'
})
export class JoinPipe implements PipeTransform {
  /**
   * Adds all the elements of an array separated by the specified
   * separator string.
   *
   * @param value The array to process.
   * @param separator A string used to separate one element of an
   *   array from the next in the resulting String. If omitted,
   *   the array elements are separated with ', '.
   */
  transform(value: Array<any>, separator?: string): string {
    return value.join(separator ? separator : ', ');
  }
}
