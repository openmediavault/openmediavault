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
import { Component, Inject } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';

import { FormSelectComponent } from '~/app/core/components/intuition/form/components/form-select/form-select.component';
import { DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-form-sharedfolder-select',
  templateUrl: './form-sharedfolder-select.component.html',
  styleUrls: ['./form-sharedfolder-select.component.scss']
})
export class FormSharedfolderSelectComponent extends FormSelectComponent {
  constructor(
    @Inject(DataStoreService) dataStoreService: DataStoreService,
    private router: Router
  ) {
    super(dataStoreService);
  }

  public onCreate(): void {
    this.router.navigate(['/storage/shared-folders/create'], {
      queryParams: { returnUrl: this.router.url }
    });
  }

  protected override sanitizeConfig(): void {
    super.sanitizeConfig();
    _.merge(this.config, {
      valueField: 'uuid',
      textField: 'description',
      placeholder: gettext('Select a shared folder ...'),
      store: {
        proxy: {
          service: 'ShareMgmt',
          get: {
            method: 'enumerateSharedFolders'
          }
        },
        sorters: [
          {
            dir: 'asc',
            prop: 'name'
          }
        ]
      }
    });
  }
}
