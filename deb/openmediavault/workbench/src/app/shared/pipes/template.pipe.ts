import { Pipe, PipeTransform } from '@angular/core';

import { renderTemplate } from '~/app/functions.helper';

@Pipe({
  name: 'template'
})
export class TemplatePipe implements PipeTransform {
  /**
   * Renders a Nunjucks/Jinja2 template.
   *
   * @param tpl The template to render.
   * @param data The object containing the data to replace
   *   the tokens.
   */
  transform(str: string, data?: Record<any, any>): string {
    return renderTemplate(str, data);
  }
}
