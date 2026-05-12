/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

export const DEFAULT_TEXTS = {
  add: gettext('Add'),
  back: gettext('Back'),
  cancel: gettext('Cancel'),
  close: gettext('Close'),
  confirmation: gettext('Confirmation'),
  create: gettext('Create'),
  delete: gettext('Delete'),
  edit: gettext('Edit'),
  information: gettext('Information'),
  no: gettext('No'),
  ok: gettext('OK'),
  save: gettext('Save'),
  start: gettext('Start'),
  stop: gettext('Stop'),
  submit: gettext('Submit'),
  yes: gettext('Yes')
} as const;
