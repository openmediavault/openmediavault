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
import { Clipboard } from '@angular/cdk/clipboard';
import { Injectable } from '@angular/core';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { NotificationService } from '~/app/shared/services/notification.service';

@Injectable({
  providedIn: 'root'
})
export class ClipboardService {
  constructor(
    private notificationService: NotificationService,
    private clipboard: Clipboard
  ) {}

  /**
   * Copy the given text to the clipboard.
   *
   * @param text The text to be copied to the clipboard.
   */
  copy(text: string): void {
    if (this.clipboard.copy(text)) {
      this.notify('success');
    } else {
      this.notify('error');
    }
  }

  private notify(type: 'success' | 'error'): void {
    const messages = {
      success: gettext('The data has been copied to the clipboard.'),
      error: gettext('Failed to copy data to the clipboard.')
    };
    this.notificationService.show(NotificationType[type], messages[type]);
  }
}
