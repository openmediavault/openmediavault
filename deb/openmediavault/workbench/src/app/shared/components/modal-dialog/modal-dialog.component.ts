import { Component, Inject } from '@angular/core';
import { MAT_DIALOG_DATA } from '@angular/material/dialog';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { Icon } from '~/app/shared/enum/icon.enum';
import { ModalDialogConfig } from '~/app/shared/models/modal-dialog-config.type';

/**
 * A dialog with a 'Yes' and 'No' button. The accept button
 * will return the boolean value 'true', the reject button 'false'.
 */
@Component({
  selector: 'omv-confirm-dialog',
  templateUrl: './modal-dialog.component.html',
  styleUrls: ['./modal-dialog.component.scss']
})
export class ModalDialogComponent {
  // Internal
  public config: ModalDialogConfig = {} as ModalDialogConfig;

  constructor(@Inject(MAT_DIALOG_DATA) data: ModalDialogConfig) {
    this.config = data;
    this.sanitizeConfig();
  }

  protected sanitizeConfig() {
    this.config.icon = _.get(Icon, this.config.icon, this.config.icon);
    switch (this.config.template) {
      case 'confirmation':
      case 'confirmation-danger':
        _.defaultsDeep(this.config, {
          title: gettext('Confirmation'),
          icon: Icon.question,
          buttons: [
            {
              text: gettext('No'),
              dialogResult: false,
              focused: true
            },
            {
              text: gettext('Yes'),
              dialogResult: true,
              class:
                this.config.template === 'confirmation-danger'
                  ? 'omv-background-color-theme-red'
                  : undefined
            }
          ]
        });
        break;
      case 'information':
        _.defaultsDeep(this.config, {
          title: gettext('Information'),
          icon: Icon.information,
          buttons: [
            {
              text: gettext('OK'),
              focused: true,
              dialogResult: true
            }
          ]
        });
        break;
    }
  }
}
