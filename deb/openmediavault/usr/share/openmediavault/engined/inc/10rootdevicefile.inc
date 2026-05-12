<?php
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
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
 */
// Make the root device file a environment variable. The root device file
// should not change under normal circumstances.
\OMV\Environment::set("OMV_ROOT_DEVICE_FILE", function() {
	// Note, the returned value is cached by the method `getRootDeviceFile`
	// itself to avoid further useless process calls. The code below is
	// executed by the omv-engined parent process, so all forked child
	// processes can access the static value as well.
	return \OMV\System\System::getRootDeviceFile();
});
