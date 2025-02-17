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

export type PageHintConfig = {
  // Depending on the type, different icons and colors are used.
  type?: 'info' | 'warning';
  // Display a close icon to allow the user to hide the hint.
  // The state is stored in the browser local storage.
  dismissible?: boolean;
  // A unique identifier, e.g. an UUIDv4, that is used to store
  // the of the hint in the browser local storage. This property
  // is required when `dismissible` is `true`.
  stateId?: string;
  // The text that is displayed.
  text: string;
};
