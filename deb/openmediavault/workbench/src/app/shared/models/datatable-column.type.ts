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
import { TableColumn } from '@siemens/ngx-datatable';

export type DatatableColumn = TableColumn & {
  // Is the column to be hidden?
  hidden?: boolean;
  /**
   * The name of the template.
   * text - Render plain text. HTML special characters like "&", "<",
   *        ">", '"', or "'" will be escaped.
   * html - Render HTML code.
   * image - Render an image. By default, the value of the configured
   *         column property is used as the image source. The `src`
   *         can be overwritten via the `cellTemplateConfig`. The
   *         `src` can be a tokenized string that will be formatted
   *         using the row values, e.g. `{{ foo }}`.
   *         {
   *           ...
   *           cellTemplateName: 'image',
   *           cellTemplateConfig: {
   *             class?: string;
   *             alt?: string;
   *             src?: string;
   *           }
   *         }
   * buttonToggle - Renders on/off toggles with the appearance of a button.
   *                {
   *                  ...
   *                  cellTemplateName: 'buttonToggle',
   *                  cellTemplateConfig: {
   *                    allowNone: boolean;
   *                    buttons: {
   *                      value: any;
   *                      text: string;
   *                    }[]
   *                  }
   *                }
   *                If `allowNone` is set to `true`, then a button
   *                can be unselected. The specified property will
   *                be set to `null` in that case. Defaults to `false`.
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
   * chip - Render the value as a chip (https://material.angular.io/components/chips/overview).
   *        If the value is a string, the optional `separator` is used to
   *        split it into substrings.
   *        The value to be rendered can be mapped. The mapped value can
   *        be a tokenized string that will be formatted using the row
   *        values.
   *        Alternatively the value to be rendered can be converted using
   *        a template.
   *        {
   *          cellTemplateName: 'chip',
   *          cellTemplateConfig: {
   *            class?: string;
   *            map?: { [key: string]: { value: string; class: string; } }
   *            template?: string;
   *            separator?: string;
   *          }
   *        }
   * binaryUnit - Convert the value into the highest possible binary unit.
   * unsortedList - Display a list of values as unsorted list.
   * copyToClipboard - Display a button to copy the cell content to the
   *                   clipboard.
   * template - Render a Nunjucks/Jinja2 template.
   * shapeShifter - Render the column based on the specified type:
   *                text - Render plain text.
   *                bold - Render text using bold font.
   *                placeholder - Display a '-' for empty values.
   *                notAvailable - Display a 'n/a' for empty values.
   *                join - Concat array values using ', '.
   *                checkIcon - Render a checkbox if value is true.
   *                progressBar - Render a progress bar.
   *                localeDateTime - Render date/time using the browser locale.
   *                relativeTime - Render relative date/time.
   * cronToHuman - Convert a Cron expression into a human-readable description.
   */
  cellTemplateName?:
    | 'text'
    | 'html'
    | 'image'
    | 'buttonToggle'
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
    | 'copyToClipboard'
    | 'cronToHuman'
    | 'template';
  cellTemplateConfig?: any; // Custom cell template configuration.
};
