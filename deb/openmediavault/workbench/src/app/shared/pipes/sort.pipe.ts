import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
    name: 'sort',
    standalone: false
})
export class SortPipe implements PipeTransform {
  transform(value: any[]): any[] {
    return value.sort();
  }
}
