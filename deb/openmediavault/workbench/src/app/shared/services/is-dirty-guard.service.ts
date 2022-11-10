/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2022 Volker Theile
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
import { Injectable } from '@angular/core';
import { CanDeactivate } from '@angular/router';
import { ActivatedRouteSnapshot, RouterStateSnapshot } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { Observable } from 'rxjs';

import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { IsDirty } from '~/app/shared/models/is-dirty.interface';
import { DialogService } from '~/app/shared/services/dialog.service';
@Injectable({
  providedIn: 'root'
})
export class IsDirtyGuardService implements CanDeactivate<IsDirty> {
  constructor(private dialogService: DialogService) {}

  canDeactivate(
    component: IsDirty,
    currentRoute: ActivatedRouteSnapshot,
    currentState: RouterStateSnapshot,
    nextState?: RouterStateSnapshot
  ): Observable<boolean> | Promise<boolean> | boolean {
    return !_.isFunction(component.isDirty) ? true : component.isDirty() ? this.showDialog() : true;
  }

  private showDialog(): Observable<boolean> {
    const dialogRef = this.dialogService.open(ModalDialogComponent, {
      data: {
        template: 'confirmation-danger',
        message: gettext(
          'You have made changes that have not yet been saved. Do you want to discard them?'
        )
      }
    });
    return dialogRef.afterClosed();
  }
}
