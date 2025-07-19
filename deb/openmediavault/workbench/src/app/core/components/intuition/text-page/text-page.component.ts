/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
import { Component, ElementRef, Inject, OnInit, Renderer2, ViewChild } from '@angular/core';
import { Router } from '@angular/router';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import * as _ from 'lodash';
import { EMPTY, exhaustMap, Observable, Subscription, timer } from 'rxjs';

import { AbstractPageComponent } from '~/app/core/components/intuition/abstract-page-component';
import {
  TextPageButtonConfig,
  TextPageConfig
} from '~/app/core/components/intuition/models/text-page-config.type';
import { PageContextService, PageStatus } from '~/app/core/services/page-context.service';
import { Unsubscribe } from '~/app/decorators';
import { Icon } from '~/app/shared/enum/icon.enum';
import { RpcObjectResponse } from '~/app/shared/models/rpc.model';
import { ClipboardService } from '~/app/shared/services/clipboard.service';
import { RpcService } from '~/app/shared/services/rpc.service';

/**
 * Display text in a read-only textarea using a non-proportional font.
 */
@Component({
  selector: 'omv-intuition-text-page',
  templateUrl: './text-page.component.html',
  styleUrls: ['./text-page.component.scss'],
  providers: [PageContextService]
})
export class TextPageComponent extends AbstractPageComponent<TextPageConfig> implements OnInit {
  @ViewChild('textContainer', { static: true })
  _textContainer: ElementRef;

  @Unsubscribe()
  private subscriptions: Subscription = new Subscription();

  protected icon = Icon;
  protected pageStatus: PageStatus;

  constructor(
    @Inject(PageContextService) pageContextService: PageContextService,
    private clipboardService: ClipboardService,
    private renderer2: Renderer2,
    private rpcService: RpcService,
    private router: Router
  ) {
    super(pageContextService);
  }

  override ngOnInit(): void {
    super.ngOnInit();
    const intervalDuration =
      _.isNumber(this.config.autoReload) && this.config.autoReload > 0
        ? this.config.autoReload
        : null;
    this.subscriptions.add(
      timer(0, intervalDuration)
        .pipe(exhaustMap(() => this.loadData()))
        .subscribe((res: any) => this.onLoadData(res))
    );
    this.subscriptions.add(
      this.pageContextService.status$.subscribe((status: PageStatus): void => {
        this.pageStatus = status;
      })
    );
  }

  onCopyToClipboard() {
    const content = this._textContainer.nativeElement.textContent;
    this.clipboardService.copy(content);
  }

  onButtonClick(buttonConfig: TextPageButtonConfig) {
    if (_.isFunction(buttonConfig.click)) {
      buttonConfig.click();
    } else {
      this.router.navigateByUrl(buttonConfig.url);
    }
  }

  onReload() {
    this.subscriptions.add(this.loadData().subscribe((res: any) => this.onLoadData(res)));
  }

  protected override doLoadData(): Observable<any> {
    const request = this.config.request;
    if (!(_.isString(request?.service) && _.isPlainObject(request?.get))) {
      return EMPTY;
    }
    return this.rpcService[request.get.task ? 'requestTask' : 'request'](
      request.service,
      request.get.method,
      request.get.params
    );
  }

  protected override onLoadData(res: any): void {
    const request = this.config.request;
    let value: any = res;
    if (_.isString(request.get.format) && RpcObjectResponse.isType(res)) {
      value = RpcObjectResponse.format(request.get.format, res);
    }
    this.renderer2.setProperty(this._textContainer.nativeElement, 'textContent', value);
  }

  protected override sanitizeConfig() {
    _.defaultsDeep(this.config, {
      autoReload: false,
      hasReloadButton: false,
      hasCopyToClipboardButton: false,
      buttonAlign: 'end',
      buttons: []
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
      }
    });
  }

  protected override onPageInit() {
    // Format tokenized configuration properties.
    this.formatConfig(['title', 'subTitle', 'request.get.method', 'request.get.params']);
  }
}
