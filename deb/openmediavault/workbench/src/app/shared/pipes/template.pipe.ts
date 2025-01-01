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

import { isFormatable, renderTemplate } from '~/app/functions.helper';

@Pipe({
  name: 'template'
})
export class TemplatePipe implements PipeTransform {
  /**
   * Renders a Nunjucks/Jinja2 template.
   *
   * @param value The template to render.
   * @param data The object containing the data to replace
   *   the tokens.
   */
  transform(value: any, data?: Record<any, any>): any {
    if (!isFormatable(value)) {
      return value;
    }
    return renderTemplate(value, data);
  }
}
