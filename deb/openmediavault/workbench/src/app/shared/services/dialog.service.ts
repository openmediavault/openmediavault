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
import { ComponentType } from '@angular/cdk/overlay';
import { Injectable } from '@angular/core';
import { MatDialog, MatDialogConfig, MatDialogRef } from '@angular/material/dialog';
import * as _ from 'lodash';

@Injectable({
  providedIn: 'root'
})
export class DialogService {
  constructor(private matDialog: MatDialog) {}

  /**
   * Close all currently open dialogs.
   */
  closeAll(): void {
    this.matDialog.closeAll();
  }

  open<T, D = any, R = any>(
    component: ComponentType<T>,
    config?: MatDialogConfig<D>
  ): MatDialogRef<T, R> {
    return this.matDialog.open(
      component,
      _.defaultsDeep(config, {
        disableClose: true
      })
    );
  }
}
