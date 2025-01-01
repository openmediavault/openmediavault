<?php
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
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
namespace OMV\System\Storage\Backend;

/**
 * Implements the storage device backend for SCSI generic character devices.
 * https://www.kernel.org/doc/Documentation/scsi/scsi-generic.txt
 * @ingroup api
 */
class SG extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_SCSIGENERIC;
	}

	function enumerate() {
		$path = "/sys/class/scsi_generic";
		if (!file_exists($path)) {
			return FALSE;
		}
		$result = [];
		foreach (new \DirectoryIterator($path) as $item) {
			if ($item->isDot() || !$item->isLink())
				continue;
			if (1 !== preg_match("/^sg[0-9]+$/", $item->getFilename()))
				continue;
			// Check if it is a SCSI generic disk.
			$sd = $this->getImpl(sprintf("/dev/%s", $item->getFilename()));
			if (0 !== $sd->getType())
				continue;
			$result[] = $sd->getDeviceFile();
		}
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/sg0
		// - /dev/sg17
		return $this->isTypeOfByName($deviceFile, "sg[0-9]+");
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceSG($args);
	}
}
