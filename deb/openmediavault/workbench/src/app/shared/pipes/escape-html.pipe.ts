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
import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'escapeHtml'
})
export class EscapeHtmlPipe implements PipeTransform {
  /**
   * Converts the characters "&", "<", ">", '"', and "'" in string to
   * their corresponding HTML entities.
   *
   * @param value The value to be converted. If it is not a string,
   *   then the value will be bypassed without converting special HTML
   *   characters to their corresponding HTML entities.
   * @return The converted value if it is a string, otherwise the
   *   origin value will be bypassed.
   */
  transform(value: any): any {
    if (_.isString(value)) {
      return _.escape(value);
    }
    return value;
  }
}
