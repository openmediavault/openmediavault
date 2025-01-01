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
import { DatatableData } from '~/app/shared/models/datatable-data.type';

export class DatatableSelection {
  selected: DatatableData[] = [];

  constructor() {}

  get length(): number {
    return this.selected.length;
  }

  get hasSelection(): boolean {
    return this.length > 0;
  }

  get hasSingleSelection(): boolean {
    return this.length === 1;
  }

  get hasMultiSelection(): boolean {
    return this.length > 1;
  }

  set(selected: DatatableData[]): void {
    this.selected = selected;
  }

  /**
   * Clear the selection.
   */
  clear(): void {
    this.selected = [];
  }

  /**
   * Get the first selection.
   */
  first(): DatatableData | null {
    if (this.hasSelection) {
      return this.selected[0];
    }
    return null;
  }
}
