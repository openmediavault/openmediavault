/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
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
