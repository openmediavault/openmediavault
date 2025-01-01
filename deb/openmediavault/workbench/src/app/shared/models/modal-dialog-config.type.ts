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
export type ModalDialogConfig = {
  // Specifies a template which pre-configures the dialog.
  // - information
  //   * Title is set to 'Information'.
  //   * Displays a 'OK' button.
  // - confirmation
  //   * Title is set to 'Confirmation'.
  //   * Displays 'Yes' and 'No' buttons.
  // - confirmation-danger
  //   * Title is set to 'Confirmation'.
  //   * Displays 'Yes' and 'No' buttons.
  //   * The 'Yes' button is marked in red.
  // - confirmation-critical
  //   * Title is set to 'Confirmation'.
  //   * Displays 'Yes' and 'No' buttons.
  //   * The 'Yes' button is marked in red.
  //   * The 'Yes' button can only be selected after a checkbox
  //     has been checked.
  template?: 'confirmation' | 'confirmation-danger' | 'confirmation-critical' | 'information';
  title?: string;
  message: string;
  icon?: string;
  // The button configurations. Will be populated automatically
  // if `template` is set.
  buttons?: Array<ModalDialogButtonConfig>;
};

export type ModalDialogButtonConfig = {
  // The text displayed in the button.
  text?: string;
  // Custom CSS class.
  class?: string;
  // Is the button disabled?
  disabled?: boolean;
  // Is the button auto-focused?
  autofocus?: boolean;
  // The dialog close input. This can be a boolean, string or
  // anything else.
  dialogResult?: any;
};
