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
export type TextPageConfig = {
  title?: string;
  subTitle?: string;
  // The frequency in milliseconds in which the data should be reloaded.
  autoReload?: boolean | number;
  hasReloadButton?: boolean;
  hasCopyToClipboardButton?: boolean;
  request?: {
    // The name of the RPC service.
    service: string;
    // The name of the RPC to get the data.
    get?: {
      // The name of the RPC.
      method: string;
      // The RPC parameters. The value can be a
      // tokenized string that will be formatted using
      // the values from the route parameters.
      // Example:
      // Configured route = '/system/certificate/ssl/detail/:foo'
      // URL = '/system/certificate/ssl/detail/abc'
      // params = { bar: '{{ foo }}' } => { bar: 'abc' }
      params?: Record<string, any>;
      // Set `true` if the RPC is a long-running background task.
      task?: boolean;
      // Convert the response object into a string.
      // Example:
      // Response = { foo: 'hello', bar: 'world' }
      // format = '{{ foo }} {{ bar }}'
      // Result = 'hello world'
      format?: string;
    };
  };
  buttonAlign?: 'start' | 'center' | 'end';
  buttons?: Array<TextPageButtonConfig>;
};

export type TextPageButtonConfig = {
  // Specifies a template that pre-configures the button:
  // back   - A button with the text 'Back'.
  // cancel - A button with the text 'Cancel'.
  template?: 'back' | 'cancel';
  // The text displayed in the button.
  text?: string;
  // Custom CSS class.
  class?: string;
  // The URL of the route to navigate to when the button has been
  // clicked.
  // Both options are mutually exclusive.
  url?: string;
  // A callback function that is called when the button has been
  // clicked. Internal only.
  click?: () => void;
};
