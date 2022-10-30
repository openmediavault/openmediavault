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
import { Component, Input, OnInit } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { Icon } from '~/app/shared/enum/icon.enum';
import { UserStorageService } from '~/app/shared/services/user-storage.service';

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

  @Input()
  hasMargin = true;

  @Input()
  dismissible = false;

  // An identifier, e.g. a UUID, which identifies this alert panel
  // uniquely. This is used to store/restore the dismissed state.
  @Input()
  stateId: string;

  // Internal
  public icon: string;
  public title: string;
  public dismissed = false;

  constructor(private userStorageService: UserStorageService) {}

  ngOnInit(): void {
    if (this.dismissible && this.stateId) {
      this.dismissed =
        'dismiss' === this.userStorageService.get(`alertpanel_state_${this.stateId}`, '');
    }
    this.sanitizeConfig();
  }

  close(): void {
    this.dismissed = true;
    if (this.stateId) {
      this.userStorageService.set(`alertpanel_state_${this.stateId}`, 'dismiss');
    }
  }

  protected sanitizeConfig() {
    this.type = _.defaultTo(this.type, 'info');
    switch (this.type) {
      case 'info':
        this.title = this.title || gettext('Information');
        this.icon = Icon.information;
        break;
      case 'success':
        this.title = this.title || gettext('Success');
        this.icon = Icon.success;
        break;
      case 'warning':
        this.title = this.title || gettext('Warning');
        this.icon = Icon.warning;
        break;
      case 'error':
        this.title = this.title || gettext('Error');
        this.icon = Icon.error;
        break;
      case 'tip':
        this.title = this.title || gettext('Tip');
        this.icon = Icon.tip;
        break;
    }
  }
}
