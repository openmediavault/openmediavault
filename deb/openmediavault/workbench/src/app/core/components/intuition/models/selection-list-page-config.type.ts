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
import { DataStore } from '~/app/shared/models/data-store.type';

export type SelectionListPageConfig = {
  title?: string;
  subTitle?: string;
  store?: DataStore;
  buttonAlign?: 'left' | 'center' | 'right';
  buttons?: Array<SelectionListPageButtonConfig>;
  multiple?: boolean;
  valueProp?: string; // Defaults to 'value'.
  textProp?: string; // Defaults to 'text'.
  // The selected values.
  value?: Array<any>;
  // If this property is set, the 'value' property will be
  // populated after the data store has been loaded. The property
  // used to evaluate the selection status should be a boolean.
  selectedProp?: string;
  // Update the store on selection change. Defaults to `false`.
  // The property 'selectedProp' is required.
  updateStoreOnSelectionChange?: boolean;
};

export type SelectionListPageButtonConfig = {
  // Specifies a template that pre-configures the button:
  // back   - A button with the text 'Back'.
  // cancel - A button with the text 'Cancel'.
  // submit - A button with the text 'Save'.
  template?: 'back' | 'cancel' | 'submit';
  // The text displayed in the button.
  text?: string;
  // Custom CSS class.
  class?: string;
  // The activity to be done when the button is pressed.
  execute?: SelectionListPageButtonExecute;
};

export type SelectionListPageButtonExecute = {
  type: 'url' | 'click';
  // The URL of the route to navigate to when the button has been
  // clicked.
  url?: string;
  // A callback function that is called when the button has been
  // clicked. Internal only.
  click?: (
    buttonConfig: SelectionListPageButtonConfig,
    store: DataStore,
    value: Array<any>
  ) => void;
};
