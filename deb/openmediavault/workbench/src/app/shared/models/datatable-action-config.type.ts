/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
import { Constraint } from '~/app/shared/models/constraint.type';
import { DataStore } from '~/app/shared/models/data-store.type';
import { Datatable } from '~/app/shared/models/datatable.interface';
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';

// A datatable action is visually represented in the action toolbar
// which is located on the left side above the datatable.
export type DatatableActionConfig = {
  // The following actions that are displayed in the datatable toolbar
  // are available:
  // button     - Displays a button with text.
  // iconbutton - Displays a button with an icon.
  // menu       - Displays a button with icon. A menu is displayed when
  //              the button is clicked. The menu items displays text
  //              and an optional icon.
  // divider    - Display a divider.
  // select     - Internal only.
  type?: 'button' | 'iconButton' | 'menu' | 'select' | 'divider';

  // An unique identifier.
  id?: string;

  // --- button | menu ---
  text?: string;

  // --- iconbutton | menu ---
  icon?: string;
  tooltip?: string;

  // --- button | iconbutton | menu ---
  // A callback function. Internal only.
  click?: (action: DatatableActionConfig, selection: DatatableSelection, table: Datatable) => void;

  // --- select ---
  store?: DataStore;
  placeholder?: string;
  // Defaults to 'value'.
  valueField?: string;
  // Defaults to 'text'.
  textField?: string;
  selectionChange?: (action: DatatableActionConfig, value: any, table: Datatable) => void;

  // --- menu ---
  // The actions displayed in the menu dropdown.
  actions?: Array<DatatableActionConfig>;

  // The constraints that must be fulfilled to enable the action.
  enabledConstraints?: {
    // The datatable must contain data?
    hasData?: boolean;
    // Minimum number of selected rows.
    minSelected?: number;
    // Maximum number of selected rows.
    maxSelected?: number;
    // Apply the specified custom constraint to all selected rows
    // (and their data). If the constraint succeeds for all selected
    // rows, then the action will be enabled.
    constraint?: Array<Constraint>;
    // A callback function. Return `true` to let the check succeed.
    callback?: (selected: any[], data: any) => boolean;
  };
};
