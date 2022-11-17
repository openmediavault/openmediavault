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

import { Directive, ViewChild } from '@angular/core';
import { Observable } from 'rxjs';

import { DatatablePageComponent } from '~/app/core/components/intuition/datatable-page/datatable-page.component';
import { FormPageComponent } from '~/app/core/components/intuition/form-page/form-page.component';
import { SelectionListPageComponent } from '~/app/core/components/intuition/selection-list-page/selection-list-page.component';
import { IsDirty } from '~/app/shared/models/is-dirty.interface';

@Directive()
export class IsDirtyFormPageComponent implements IsDirty {
  @ViewChild(FormPageComponent, { static: true })
  protected page: FormPageComponent;

  isDirty(): Observable<boolean> | Promise<boolean> | boolean {
    return this.page?.isDirty?.() ?? false;
  }
}

@Directive()
export class IsDirtySelectionListPageComponent implements IsDirty {
  @ViewChild(SelectionListPageComponent, { static: true })
  protected page: SelectionListPageComponent;

  isDirty(): Observable<boolean> | Promise<boolean> | boolean {
    return this.page?.isDirty?.() ?? false;
  }
}

@Directive()
export class IsDirtyDatatablePageComponent implements IsDirty {
  @ViewChild(DatatablePageComponent, { static: true })
  protected page: DatatablePageComponent;

  protected dirty = false;

  isDirty(): Observable<boolean> | Promise<boolean> | boolean {
    return this.dirty;
  }
}
