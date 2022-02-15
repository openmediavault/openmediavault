import { Pipe, PipeTransform } from '@angular/core';
import * as _ from 'lodash';

@Pipe({
  name: 'preventHtml'
})
export class PreventHtmlPipe implements PipeTransform {
  /**
   * This pipe will return `null` if the content contains HTML tags.
   */
  transform(value: any): any {
    if (_.isString(value) && this.isHtml(value)) {
      return null;
    }
    return value;
  }

  private isHtml(text: string): boolean {
    // Keep it simple here; we don't want to use an external NPM package
    // for such a trivial thing.
    // Note, it is not expected that this pipe has to process very complex
    // content.
    // Regular expression was found at https://stackoverflow.com/a/15459273.
    // eslint-disable-next-line max-len
    return /<(br|basefont|hr|input|source|frame|param|area|meta|!--|col|link|option|base|img|wbr|!DOCTYPE).*?>|<(a|abbr|acronym|address|applet|article|aside|audio|b|bdi|bdo|big|blockquote|body|button|canvas|caption|center|cite|code|colgroup|command|datalist|dd|del|details|dfn|dialog|dir|div|dl|dt|em|embed|fieldset|figcaption|figure|font|footer|form|frameset|head|header|hgroup|h1|h2|h3|h4|h5|h6|html|i|iframe|ins|kbd|keygen|label|legend|li|map|mark|menu|meter|nav|noframes|noscript|object|ol|optgroup|output|p|pre|progress|q|rp|rt|ruby|s|samp|script|section|select|small|span|strike|strong|style|sub|summary|sup|table|tbody|td|textarea|tfoot|th|thead|time|title|tr|track|tt|u|ul|var|video).*?<\/\2>/i.test(
      text
    );
  }
}
