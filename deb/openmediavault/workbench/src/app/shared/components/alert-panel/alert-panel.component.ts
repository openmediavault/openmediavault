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
