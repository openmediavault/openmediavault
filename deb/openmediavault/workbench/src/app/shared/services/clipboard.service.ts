import { Platform } from '@angular/cdk/platform';
import { Injectable } from '@angular/core';
import { marker as gettext } from '@biesbjerg/ngx-translate-extract-marker';
import * as _ from 'lodash';

import { NotificationType } from '~/app/shared/enum/notification-type.enum';
import { NotificationService } from '~/app/shared/services/notification.service';

@Injectable({
  providedIn: 'root'
})
export class ClipboardService {
  constructor(private notificationService: NotificationService, private platform: Platform) {}

  /**
   * Check if the Clipboard API exists and copy to clipboard is possible.
   * The following prerequisites have to be fulfilled:
   * - The HTTPS protocol is used. localhost is also supported.
   * - The site is not embedded in an iFrame.
   */
  canCopy(): boolean {
    return !_.isUndefined(navigator.clipboard);
  }

  /**
   * Copy the given text to the clipboard.
   *
   * @param text The text to be copied to the clipboard.
   */
  copy(text: string): void {
    // Use the Clipboard API.
    // https://developer.mozilla.org/en-US/docs/Web/API/Clipboard
    // https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/Interact_with_the_clipboard
    if (!this.canCopy()) {
      return;
    }
    // Various browsers do not support the `Permissions API`.
    // https://developer.mozilla.org/en-US/docs/Web/API/Permissions_API#Browser_compatibility
    if (
      !navigator.permissions ||
      this.platform.FIREFOX ||
      this.platform.SAFARI ||
      this.platform.IOS ||
      this.platform.TRIDENT
    ) {
      this.writeText(text);
    } else {
      navigator.permissions
        .query({ name: 'clipboard-write' as PermissionName })
        .then((ps: PermissionStatus) => {
          if (_.includes(['granted', 'prompt'], ps.state)) {
            this.writeText(text);
          } else if ('denied' === ps.state) {
            this.notify('denied');
          }
        });
    }
  }

  private notify(type: 'success' | 'error' | 'denied'): void {
    const messages = {
      success: gettext('The data has been copied to the clipboard.'),
      error: gettext('Failed to write data to the clipboard.'),
      denied: gettext('Permission denied to write data to the clipboard.')
    };
    this.notificationService.show(NotificationType.success, messages[type]);
  }

  private writeText(text): void {
    navigator.clipboard
      .writeText(text)
      .then(() => this.notify('success'))
      .catch(() => this.notify('error'));
  }
}
