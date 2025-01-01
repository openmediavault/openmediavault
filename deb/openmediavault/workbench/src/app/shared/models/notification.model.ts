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
import { NotificationType } from '~/app/shared/enum/notification-type.enum';

export class Notification {
  private static nextId = 1;

  public readonly id: number;
  public timestamp: string;

  constructor(
    public type: NotificationType = NotificationType.info,
    public title?: string,
    public message?: string,
    public traceback?: string,
    public url?: string,
    public dismissible: boolean = true
  ) {
    this.id = Notification.nextId++;
    this.timestamp = new Date().toJSON(); // ISO 8601
  }
}
