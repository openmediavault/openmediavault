import { TableColumn } from '@swimlane/ngx-datatable';

export type DatatableColumn = TableColumn & {
  // Is the column to be hidden?
  hidden?: boolean;
  /**
   * The name of the template.
   * bold - Render text using bold font.
   * buttonToogle - Renders on/off toggles with the appearance of a button.
   *                {
   *                  ...
   *                  cellTemplateName: 'buttonToogle',
   *                  cellTemplateConfig: Array<{
   *                    value: string;
   *                    text: string;
   *                  }>
   *                }
   * checkIcon - Render a checkbox if value is true.
   * placeholder - Display a '-' for empty values.
   * progressBar - Render a progress bar.
   * notAvailable - Display a 'n/a' for empty values.
   * join - Concat array values using ', '.
   * truncate - Truncates string if it's longer than the given maximum
   *            string length.
   *            {
   *              ...
   *              cellTemplateName: 'truncate',
   *              cellTemplateConfig: {
   *                length: number;
   *                omission?: string;
   *                separator?: string;
   *              }
   *            }
   * code - Render value as preformatted text using a non-proportional font.
   * localeDateTime - Render date/time using the browser locale.
   * relativeTime - Render relative time like '2 minutes ago'.
   * chip - Render the value as chip (https://material.angular.io/components/chips/overview).
   *        The value to be rendered can be mapped. The mapped value can
   *        be a tokenized string that will be formatted using the row
   *        values.
   *        Alternatively the value to be rendered can converted using
   *        a template.
   *        {
   *          cellTemplateName: 'chip',
   *          cellTemplateConfig: {
   *            class?: string;
   *            map?: { [key: string]: { value: string; class: string; } }
   *            template?: string;
   *          }
   *        }
   * binaryUnit - Convert the value into the highest possible binary unit.
   * unsortedList - Display a list of values as unsorted list.
   * template - Render a Nunjucks/Jinja2 template.
   * shapeShifter - Render the column based on the specified type:
   *                text - Plain text.
   *                bold - Render text using bold font.
   *                placeholder - Display a '-' for empty values.
   *                notAvailable - Display a 'n/a' for empty values.
   *                join - Concat array values using ', '.
   *                checkIcon - Render a checkbox if value is true.
   *                progressBar - Render a progress bar.
   *                localeDateTime - Render date/time using the browser locale.
   *                relativeTime - Render relative date/time.
   */
  cellTemplateName?:
    | 'buttonToogle'
    | 'checkIcon'
    | 'placeholder'
    | 'progressBar'
    | 'notAvailable'
    | 'join'
    | 'truncate'
    | 'shapeShifter'
    | 'localeDateTime'
    | 'relativeTime'
    | 'chip'
    | 'binaryUnit'
    | 'unsortedList'
    | 'template';
  cellTemplateConfig?: any; // Custom cell template configuration.
};
