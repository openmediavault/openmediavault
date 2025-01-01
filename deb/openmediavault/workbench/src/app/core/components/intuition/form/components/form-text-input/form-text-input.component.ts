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
import { Component, OnInit, ViewChild } from '@angular/core';
import { AbstractControl } from '@angular/forms';
import { MatAutocompleteTrigger } from '@angular/material/autocomplete';
import * as _ from 'lodash';
import { Observable, throwError } from 'rxjs';
import { catchError, map, skipWhile, startWith } from 'rxjs/operators';

import { AbstractFormFieldComponent } from '~/app/core/components/intuition/form/components/abstract-form-field-component';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { DataStoreService } from '~/app/shared/services/data-store.service';

@Component({
  selector: 'omv-form-text-input',
  templateUrl: './form-text-input.component.html',
  styleUrls: ['./form-text-input.component.scss']
})
export class FormTextInputComponent extends AbstractFormFieldComponent implements OnInit {
  @ViewChild(MatAutocompleteTrigger)
  autocompleteTrigger: MatAutocompleteTrigger;

  public loading = false;
  public filteredOptions$: Observable<Record<string, any>>;

  private abstractControl: AbstractControl;

  constructor(
    private clipboardService: ClipboardService,
    private dataStoreService: DataStoreService
  ) {
    super();
  }

  override ngOnInit(): void {
    super.ngOnInit();
    this.abstractControl = this.formGroup.get(this.config.name);
    this.filteredOptions$ = this.abstractControl.valueChanges.pipe(
      startWith(this.abstractControl.value),
      skipWhile(() => this.loading),
      map((value) => {
        return this.doFilter(value ?? '');
      })
    );
    if (this.config.suggestions) {
      this.doLoadStore();
    }
  }

  onCopyToClipboard(): void {
    this.clipboardService.copy(this.abstractControl.value);
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
        // Update `filteredOptions$`.
        this.abstractControl.updateValueAndValidity();
      });
  }

  private doFilter(value: string): Record<string, any> {
    return _.filter(this.config.store.data, (option) => {
      return _.toLower(option[this.config.valueField]).includes(_.toLower(value));
    });
  }
}
