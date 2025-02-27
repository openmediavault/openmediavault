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
import { Injectable } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { Subscription } from 'rxjs';
import { distinctUntilChanged, map } from 'rxjs/operators';

import { Unsubscribe } from '~/app/decorators';
import {
  SystemInformation,
  SystemInformationService
} from '~/app/shared/services/system-information.service';

const DEFAULT_TITLE = 'openmediavault Workbench';

@Injectable({
  providedIn: 'root'
})
export class TitleService {
  @Unsubscribe()
  private subscriptions = new Subscription();

  constructor(
    private systemInformationService: SystemInformationService,
    private title: Title
  ) {
    this.subscriptions.add(
      this.systemInformationService.systemInfo$
        .pipe(
          map((resp: SystemInformation): string => resp.hostname),
          distinctUntilChanged()
        )
        .subscribe((hostname: string) => {
          const newTitle = `${hostname} - ${DEFAULT_TITLE}`;
          this.title.setTitle(newTitle);
        })
    );
  }
}
