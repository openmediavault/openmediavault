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
import { Component, OnInit } from '@angular/core';
import { MatSelectChange } from '@angular/material/select';
import * as _ from 'lodash';
import { Subscription, throwError } from 'rxjs';
import { catchError } from 'rxjs/operators';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { Unsubscribe } from '~/app/decorators';
import { DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-form-select',
  templateUrl: './form-select.component.html',
  styleUrls: ['./form-select.component.scss']
})
export class FormSelectComponent extends AbstractFormFieldComponent implements OnInit {
  @Unsubscribe()
  private subscriptions = new Subscription();

  public loading = false;

  constructor(private dataStoreService: DataStoreService) {
    super();
  }

  override ngOnInit(): void {
    super.ngOnInit();
    this.doLoadStore();

    const control = this.formGroup.get(this.config.name);
    this.subscriptions.add(
      control.valueChanges.subscribe((value) => {
        this.config.value = value;
      })
    );
  }

  onSelectionChange(event: MatSelectChange): void {
    if (_.isFunction(this.config.selectionChange)) {
      const value = _.clone(event.value);
      this.config.selectionChange(value);
    }
  }

  private doLoadStore(): void {
    this.loading = true;
    this.dataStoreService
      .load(this.config.store)
      .pipe(
        catchError((error) => {
          this.loading = false;
          return throwError(error);
        })
      )
      .subscribe(() => {
        this.loading = false;
        if (this.config.hasEmptyOption) {
          const item = {};
          _.set(item, this.config.valueField, '');
          _.set(item, this.config.textField, this.config.emptyOptionText);
          this.config.store.data.unshift(item);
        }
      });
  }
}
