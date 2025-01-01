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
 * Implements the storage device backend for Linux Bcache devices.
 * @see https://www.kernel.org/doc/Documentation/bcache.txt
 * @ingroup api
 */
class Bcache extends BackendAbstract {
	protected $regex = "bcache[0-9]+";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if (FALSE === $devs)
			return FALSE;
		$result = [];
		foreach($devs as $devk => $devv) {
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			$slaves = $object->getSlaves();
			if (FALSE === $slaves)
				continue;
			$result = array_merge($result, $slaves);
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceBcache($args);
	}

	function fsDeviceFile($deviceFile, $partition = 1) {
		return $deviceFile;
	}
}
