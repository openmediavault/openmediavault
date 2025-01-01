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
import { Component, ElementRef, OnDestroy, OnInit } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import * as _ from 'lodash';

import { BlockUiService } from '~/app/shared/services/block-ui.service';
import { DialogService } from '~/app/shared/services/dialog.service';

@Component({
  selector: 'omv-guru-meditation-page',
  templateUrl: './guru-meditation-page.component.html',
  styleUrls: ['./guru-meditation-page.component.scss']
})
export class GuruMeditationPageComponent implements OnInit, OnDestroy {
  // https://web.archive.org/web/20120628060356/http://www.scotek.demon.co.uk/guru.html
  // https://www.amigalove.com/viewtopic.php?t=500
  // https://github.com/deplinenoise/amiga-sdk/blob/master/sdkinclude/exec/alerts.i
  message = 'Guru Meditation #31000000.48454C50';
  url = '/';

  constructor(
    private activatedRoute: ActivatedRoute,
    private blockUiService: BlockUiService,
    private dialogService: DialogService,
    private elementRef: ElementRef,
    private router: Router
  ) {
    _.forEach(['message', 'url'], (queryParam: string) => {
      _.forEach(['routeConfig.data', 'snapshot.queryParams'], (path: string) => {
        if (_.hasIn(this.activatedRoute, `${path}.${queryParam}`)) {
          _.set(this, queryParam, _.get(this.activatedRoute, `${path}.${queryParam}`));
          return false; // Abort loop.
        }
        return true;
      });
    });
  }

  ngOnInit(): void {
    this.blockUiService.resetGlobal();
    // Ensure all currently opened dialogs are closed.
    this.dialogService.closeAll();
    this.elementRef.nativeElement.addEventListener('click', this.onClick.bind(this));
  }

  ngOnDestroy(): void {
    this.elementRef.nativeElement.removeEventListener('click', this.onClick);
  }

  private onClick() {
    let url = this.url;
    // Sanitize the URL so that it does not refer to itself.
    if (['/404', '/503'].includes(url)) {
      url = '/';
    }
    this.router.navigate([url]);
  }
}
