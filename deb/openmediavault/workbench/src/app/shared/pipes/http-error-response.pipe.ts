import { HttpErrorResponse } from '@angular/common/http';
import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'httpErrorResponse'
})
export class HttpErrorResponsePipe implements PipeTransform {
  /**
   * Extract the specified property from the given HttpErrorResponse
   * object.
   *
   * @param value The HttpErrorResponse object.
   * @param output The specified property.
   */
  transform(value: HttpErrorResponse, output?: any): any {
    let result: any;
    switch (output) {
      case 'statusText':
        result = _.get(value, 'statusText', '');
        break;
      case 'url':
        result = value.url;
        break;
      case 'status':
        result = value.status;
        break;
      case 'trace':
        if (_.isObjectLike(value) && _.isObjectLike(value.error)) {
          result = _.get(value.error, 'trace', '');
        }
        break;
      case 'message':
      default:
        if (_.isObjectLike(value) && _.isObjectLike(value.error)) {
          result = _.get(value.error, 'message', '');
        } else {
          result = _.get(value, 'message', '');
        }
        break;
    }
    return result;
  }
}
