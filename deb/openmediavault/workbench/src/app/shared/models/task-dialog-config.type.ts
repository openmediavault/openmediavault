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
export type TaskDialogConfig = {
  // A title within the header.
  title?: string;
  // An image displayed within the header.
  icon?: string;
  // Width of the dialog (in px or percent).
  width?: string;
  // Set to `false` to do not automatically scroll down the
  // content. Defaults to `true`.
  autoScroll?: boolean;
  // Start the action on dialog initialization. If set to `true`,
  // then the 'Start' button gets hidden. Defaults to `false`.
  startOnInit?: boolean;
  // Display a message when the request has been completed?
  // Defaults to `true`.
  showCompletion?: boolean;
  // The custom button configurations. They are merged with the
  // default settings.
  // The defaults are:
  // - start:
  //   - The button text defaults to `Start`.
  //   - The button is visible by default. The button gets hidden
  //     automatically if `startOnInit` is set to `true`.
  //   - The button is not disabled by default.
  //   - The button is not focused by default.
  // - stop:
  //   - The button text defaults to `Stop`.
  //   - The button is visible by default.
  //   - The button is disabled by default.
  //   - The button is not focused by default.
  // - close:
  //   - The button text defaults to `Close`.
  //   - The button is visible by default.
  //   - The button is disabled by default.
  //   - The button is focused by default.
  //   - The value submitted when the dialog gets closed defaults
  //     to `false`. If the request finished successfully, it will
  //     be set to `true`.
  buttons?: {
    start?: TaskDialogButtonConfig;
    stop?: TaskDialogButtonConfig;
    close?: TaskDialogButtonConfig;
  };
  request?: {
    // The name of the RPC service.
    service: string;
    // The name of the RPC. This RPC must return the file name that
    // is used to communicate with the background task.
    method: string;
    // Additional parameters.
    params?: Record<string, any>;
    // Number of retry attempts before failing.
    maxRetries?: number;
  };
};

export type TaskDialogButtonConfig = {
  // The text displayed in the button.
  text?: string;
  // Custom CSS class.
  class?: string;
  // Is the button hidden?
  hidden?: boolean;
  // Is the button disabled?
  disabled?: boolean;
  // Is the button auto-focused?
  autofocus?: boolean;
  // The dialog close input. This can be a boolean, string or
  // anything else. Internal only.
  dialogResult?: any;
};
