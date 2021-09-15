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
import { HttpErrorResponse } from '@angular/common/http';
import { Component, Inject, OnInit, ViewChild } from '@angular/core';
import { MatSelectionList, MatSelectionListChange } from '@angular/material/list';
import { ActivatedRoute, Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { EMPTY } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { AbstractPageComponent } from '~/app/core/components/intuition/abstract-page-component';
import {
  SelectionListPageButtonConfig,
  SelectionListPageConfig
} from '~/app/core/components/intuition/models/selection-list-page-config.type';
import { format, toBoolean } from '~/app/functions.helper';
import { ModalDialogComponent } from '~/app/shared/components/modal-dialog/modal-dialog.component';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { DataStore } from '~/app/shared/models/data-store.type';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { DataStoreService } from '~/app/shared/services/data-store.service';
import { DialogService } from '~/app/shared/services/dialog.service';
import { NotificationService } from '~/app/shared/services/notification.service';

@Component({
  selector: 'omv-intuition-selection-list-page',
  templateUrl: './selection-list-page.component.html',
  styleUrls: ['./selection-list-page.component.scss']
})
export class SelectionListPageComponent
  extends AbstractPageComponent<SelectionListPageConfig>
  implements OnInit
{
  @ViewChild('list', { static: true })
  list: MatSelectionList;

  public error: HttpErrorResponse;
  public loading = false;
  public pristine = true;
  public data: Record<string, any>[] = [];

  constructor(
    @Inject(ActivatedRoute) activatedRoute: ActivatedRoute,
    @Inject(AuthSessionService) authSessionService: AuthSessionService,
    private dataStoreService: DataStoreService,
    private dialogService: DialogService,
    private notificationService: NotificationService,
    private router: Router
  ) {
    super(activatedRoute, authSessionService);
  }

  ngOnInit(): void {
    super.ngOnInit();
    this.loadData();
  }

  onSelectionChange(event: MatSelectionListChange) {
    if (!_.isEmpty(this.config.selectedProp) && this.config.updateStoreOnSelectionChange) {
      const index = _.findIndex(this.config.store.data, [
        this.config.valueProp,
        event.options[0].value
      ]);
      if (-1 !== index) {
        const item: Record<string, any> = _.nth(this.config.store.data, index);
        _.set(item, this.config.selectedProp, event.options[0].selected);
        this.pristine = false;
      }
    }
  }

  onButtonClick(buttonConfig: SelectionListPageButtonConfig): void {
    const doButtonActionFn = () => {
      switch (buttonConfig?.execute?.type) {
        case 'click':
          if (_.isFunction(buttonConfig.execute.click)) {
            const values = [];
            _.forEach(this.list.selectedOptions.selected, (selected) => {
              values.push(selected.value);
            });
            buttonConfig.execute.click(buttonConfig, this.config.store, values);
          }
          break;
        case 'url':
          if (!_.isEmpty(buttonConfig.execute.url)) {
            this.router.navigate([buttonConfig.execute.url]);
          }
          break;
      }
    };
    if (!this.pristine && ['back', 'cancel'].includes(buttonConfig?.template)) {
      const dialogRef = this.dialogService.open(ModalDialogComponent, {
        data: {
          template: 'confirmation-danger',
          message: gettext('Your changes will be lost. Do you want to continue?')
        }
      });
      dialogRef.afterClosed().subscribe((res: any) => {
        if (true === res) {
          doButtonActionFn();
        }
      });
    } else {
      doButtonActionFn();
    }
  }

  onSubmit(buttonConfig: SelectionListPageButtonConfig, store: DataStore, value: Array<any>) {
    this.dataStoreService
      .save(store)
      .pipe(
        catchError((error) => {
          this.error = error;
          return EMPTY;
        })
      )
      .subscribe(() => {
        // Display the configured notification message.
        const notificationTitle = _.get(this.routeConfig, 'data.notificationTitle');
        if (!_.isEmpty(notificationTitle)) {
          this.notificationService.show(
            NotificationType.success,
            format(notificationTitle, this.pageContext)
          );
        }
        // Navigate to an optional URL.
        if (!_.isEmpty(buttonConfig.execute.url)) {
          this.router.navigate([buttonConfig.execute.url]);
        }
      });
  }

  protected sanitizeConfig() {
    _.defaultsDeep(this.config, {
      buttonAlign: 'end',
      buttons: [],
      multiple: false,
      valueField: 'value',
      textField: 'text',
      selectedField: undefined,
      updateInline: false,
      value: []
    });
    // Set the default values of the buttons.
    _.forEach(this.config.buttons, (button) => {
      const template = _.get(button, 'template');
      switch (template) {
        case 'back':
          _.defaultsDeep(button, {
            text: gettext('Back')
          });
          break;
        case 'cancel':
          _.defaultsDeep(button, {
            text: gettext('Cancel')
          });
          break;
        case 'submit':
          _.defaultsDeep(button, {
            text: gettext('Save'),
            execute: {
              type: 'click',
              click: this.onSubmit.bind(this)
            }
          });
          break;
      }
    });
    // Relocate the 'submit' button to the end of the list.
    const index = _.findIndex(this.config.buttons, ['template', 'submit']);
    if (index !== -1) {
      const button = this.config.buttons[index];
      this.config.buttons.splice(index, 1);
      this.config.buttons.push(button);
    }
  }

  private loadData() {
    const store = this.config.store;
    this.loading = true;
    this.dataStoreService
      .load(store)
      .pipe(
        catchError((error) => {
          this.error = error;
          return EMPTY;
        }),
        finalize(() => {
          this.loading = false;
        })
      )
      .subscribe(
        () => {
          // Extract the selected values from the loaded data.
          if (!_.isEmpty(this.config.selectedProp)) {
            const value = [];
            _.forEach(this.config.store.data, (item) => {
              const selectedFieldValue = _.get(item, this.config.selectedProp, false);
              if (toBoolean(selectedFieldValue)) {
                value.push(_.get(item, this.config.valueProp));
              }
            });
            this.config.value = value;
          }
        },
        () => {
          store.data = [];
        }
      );
  }
}
