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
   * Copy the given text to the clipboard.
   *
   * @param text The text to be copied to the clipboard.
   */
  copy(text: string): void {
    const notificationFn = () => {
      this.notificationService.show(
        NotificationType.success,
        gettext('The data has been copied to the clipboard.')
      );
    };
    if (
      this.platform.FIREFOX ||
      this.platform.SAFARI ||
      this.platform.IOS ||
      this.platform.TRIDENT
    ) {
      // Various browsers do not support the `Permissions API`.
      // https://developer.mozilla.org/en-US/docs/Web/API/Permissions_API#Browser_compatibility
      navigator.clipboard.writeText(text).then(() => notificationFn());
    } else {
      navigator.permissions
        .query({ name: 'clipboard-write' as PermissionName })
        .then((ps: PermissionStatus) => {
          if (_.includes(['granted', 'prompt'], ps.state)) {
            navigator.clipboard.writeText(text).then(() => notificationFn());
          }
        });
    }
  }
}
