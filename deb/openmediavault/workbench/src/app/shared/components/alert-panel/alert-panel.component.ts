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
import { Component, EventEmitter, HostBinding, Input, OnInit, Output } from '@angular/core';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';

import { CoerceBoolean } from '~/app/decorators';
import { Icon } from '~/app/shared/enum/icon.enum';
import { UserLocalStorageService } from '~/app/shared/services/user-local-storage.service';

export type AlertPanelButtonConfig = {
  icon: string;
  class?: string;
  tooltip?: string;
  click?: (config: AlertPanelButtonConfig) => void;
};

@Component({
  selector: 'omv-alert-panel',
  templateUrl: './alert-panel.component.html',
  styleUrls: ['./alert-panel.component.scss']
})
export class AlertPanelComponent implements OnInit {
  @Input()
  type: 'info' | 'success' | 'warning' | 'error' | 'tip';

  @CoerceBoolean()
  @Input()
  hasTitle = true;

  @CoerceBoolean()
  @Input()
  hasMargin = true;

  @CoerceBoolean()
  @Input()
  dismissible = false;

  // An identifier, e.g. a UUID, which identifies this alert panel
  // uniquely. This is used to store/restore the dismissed state.
  @Input()
  stateId: string;

  @Input()
  buttons: AlertPanelButtonConfig[] = [];

  @Input()
  icon?: string;

  @Input()
  title?: string;

  @Output()
  closed = new EventEmitter();

  // Internal
  public dismissed = false;

  constructor(private userLocalStorageService: UserLocalStorageService) {}

  @HostBinding('class')
  get class(): string {
    const result: string[] = [];
    if (this.dismissed) {
      result.push('omv-display-none');
    }
    return result.join(' ');
  }

  ngOnInit(): void {
    if (this.dismissible) {
      this.buttons.push({
        icon: Icon.close,
        tooltip: gettext('Dismiss'),
        click: this.close.bind(this)
      });

      if (this.stateId) {
        this.dismissed =
          'dismiss' === this.userLocalStorageService.get(`alertpanel_state_${this.stateId}`, '');
      }
    }
    this.sanitizeConfig();
  }

  close(): void {
    this.dismissed = true;
    if (this.stateId) {
      this.userLocalStorageService.set(`alertpanel_state_${this.stateId}`, 'dismiss');
    }
    this.closed.emit();
  }

  protected sanitizeConfig() {
    this.type = _.defaultTo(this.type, 'info');
    switch (this.type) {
      case 'info':
        this.title = this.title || gettext('Information');
        this.icon = _.get(Icon, this.icon, Icon.information);
        break;
      case 'success':
        this.title = this.title || gettext('Success');
        this.icon = _.get(Icon, this.icon, Icon.success);
        break;
      case 'warning':
        this.title = this.title || gettext('Warning');
        this.icon = _.get(Icon, this.icon, Icon.warning);
        break;
      case 'error':
        this.title = this.title || gettext('Error');
        this.icon = _.get(Icon, this.icon, Icon.error);
        break;
      case 'tip':
        this.title = this.title || gettext('Tip');
        this.icon = _.get(Icon, this.icon, Icon.tip);
        break;
    }
  }
}
