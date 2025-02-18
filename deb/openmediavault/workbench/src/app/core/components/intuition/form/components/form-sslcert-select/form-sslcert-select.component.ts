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
import * as _ from 'lodash';

import { FormSelectComponent } from '~/app/core/components/intuition/form/components/form-select/form-select.component';
import { DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-form-sslcert-select',
  templateUrl: './form-sslcert-select.component.html',
  styleUrls: ['./form-sslcert-select.component.scss']
})
export class FormSslcertSelectComponent extends FormSelectComponent {
  constructor(
    @Inject(DataStoreService) dataStoreService: DataStoreService,
    private router: Router
  ) {
    super(dataStoreService);
  }

  public onCreate(): void {
    this.router.navigate(['/system/certificate/ssl/create'], {
      queryParams: { returnUrl: this.router.url }
    });
  }

  protected override sanitizeConfig(): void {
    super.sanitizeConfig();
    _.merge(this.config, {
      valueField: 'uuid',
      textField: 'comment',
      placeholder: 'Select a SSL certificate ...',
      store: {
        proxy: {
          service: 'CertificateMgmt',
          get: {
            method: 'getList',
            params: {
              start: 0,
              limit: -1
            }
          }
        },
        sorters: [
          {
            dir: 'asc',
            field: 'name'
          }
        ]
      }
    });
  }
}
