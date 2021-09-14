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
              autofocus: true
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
              autofocus: true,
              dialogResult: true
            }
          ]
        });
        break;
    }
  }
}
