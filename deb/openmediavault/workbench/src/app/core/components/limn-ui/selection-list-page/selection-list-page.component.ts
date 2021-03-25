import { HttpErrorResponse } from '@angular/common/http';
import { Component, Inject, OnInit, ViewChild } from '@angular/core';
import { MatSelectionList, MatSelectionListChange } from '@angular/material/list';
import { ActivatedRoute, Router } from '@angular/router';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';
import { EMPTY } from 'rxjs';
import { catchError, finalize } from 'rxjs/operators';

import { AbstractPageComponent } from '~/app/core/components/limn-ui/abstract-page-component';
import {
  SelectionListPageButtonConfig,
  SelectionListPageConfig
} from '~/app/core/components/limn-ui/models/selection-list-page-config.type';
import { format, toBoolean } from '~/app/functions.helper';
import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { DataStore } from '~/app/shared/models/data-store.type';
import { AuthSessionService } from '~/app/shared/services/auth-session.service';
import { DataStoreService } from '~/app/shared/services/data-store.service';
import { NotificationService } from '~/app/shared/services/notification.service';

@Component({
  selector: 'omv-limn-selection-list-page',
  templateUrl: './selection-list-page.component.html',
  styleUrls: ['./selection-list-page.component.scss']
})
export class SelectionListPageComponent
  extends AbstractPageComponent<SelectionListPageConfig>
  implements OnInit {
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
      buttonAlign: 'left',
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
