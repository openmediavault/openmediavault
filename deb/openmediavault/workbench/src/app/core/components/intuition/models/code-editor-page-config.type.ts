import { TextPageButtonConfig } from '~/app/core/components/intuition/models/text-page-config.type';

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
export type CodeEditorPageConfig = {
  title?: string;
  subTitle?: string;
  // The frequency in milliseconds in which the data should be reloaded.
  autoReload?: boolean | number;
  hasReloadButton?: boolean;
  hasCopyToClipboardButton?: boolean;
  // The language of the code editor. Defaults to `none`.
  language?: 'json' | 'python' | 'shell' | 'xml' | 'yaml' | 'none';
  // Display line numbers in the code editor? Defaults to `true`.
  lineNumbers?: boolean;
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
  buttons?: Array<CodeEditorPageButtonConfig>;
};

export type CodeEditorPageButtonConfig = TextPageButtonConfig;
