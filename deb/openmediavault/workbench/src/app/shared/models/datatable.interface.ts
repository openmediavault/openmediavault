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
import { DatatableSelection } from '~/app/shared/models/datatable-selection.model';

/**
 * A simple interface to interact with the data table.
 */
export interface Datatable {
  // The data shown in the table.
  data: DatatableData[];

  // The current selection.
  selection: DatatableSelection;

  /**
   * Reload the data to be shown.
   */
  reloadData(): void;

  /**
   * Update the data to be shown.
   */
  updateData(data: DatatableData[]): void;
}
