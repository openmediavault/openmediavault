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
export class DatatableSelection {
  selected: Array<any> = [];
  hasSelection: boolean;
  hasSingleSelection: boolean;
  hasMultiSelection: boolean;

  constructor() {
    this.update();
  }

  set(selected: any[]) {
    this.selected = selected;
    this.update();
  }

  /**
   * Clear the selection.
   */
  clear() {
    this.selected = [];
    this.update();
  }

  /**
   * Update the internal data structure.
   */
  update() {
    this.hasSelection = this.selected.length > 0;
    this.hasSingleSelection = this.selected.length === 1;
    this.hasMultiSelection = this.selected.length > 1;
  }

  /**
   * Get the first selection.
   */
  first() {
    if (this.hasSelection) {
      return this.selected[0];
    }
    return null;
  }
}
