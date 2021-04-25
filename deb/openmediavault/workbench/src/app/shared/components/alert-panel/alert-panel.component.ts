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
import { Component, Input, OnInit } from '@angular/core';
import * as _ from 'lodash';

import { translate } from '~/app/i18n.helper';
import { Icon } from '~/app/shared/enum/icon.enum';

@Component({
  selector: 'omv-alert-panel',
  templateUrl: './alert-panel.component.html',
  styleUrls: ['./alert-panel.component.scss']
})
export class AlertPanelComponent implements OnInit {
  @Input()
  type: 'info' | 'success' | 'warning' | 'error' | 'tip';

  @Input()
  hasTitle = true;

  // Internal
  public icon: string;
  public title: string;

  constructor() {}

  ngOnInit(): void {
    this.sanitizeConfig();
  }

  protected sanitizeConfig() {
    this.type = _.defaultTo(this.type, 'info');
    switch (this.type) {
      case 'info':
        this.title = this.title || translate('Information');
        this.icon = Icon.information;
        break;
      case 'success':
        this.title = this.title || translate('Success');
        this.icon = Icon.success;
        break;
      case 'warning':
        this.title = this.title || translate('Warning');
        this.icon = Icon.warning;
        break;
      case 'error':
        this.title = this.title || translate('Error');
        this.icon = Icon.error;
        break;
      case 'tip':
        this.title = this.title || translate('Tip');
        this.icon = Icon.tip;
        break;
    }
  }
}
