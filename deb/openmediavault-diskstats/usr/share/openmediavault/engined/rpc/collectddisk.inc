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
namespace Engined\Rpc;

class OMVRpcServiceCollectdDisk extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "CollectdDisk";
	}

	public function initialize() {
		$this->registerMethod("enumerate");
	}

	public function enumerate($params, $context) {
		$objects = \OMV\Rpc\Rpc::call("FileSystemMgmt",
			"enumerateMountedFilesystems", [
				"includeroot" => TRUE
			], $context);
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			if (empty($objectv['parentdevicefile'])) {
				continue;
			}
			$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
				$objectv['parentdevicefile']);
			$result[] = [
				"devicefile" => $sd->getPreferredDeviceFile(),
				"predictabledevicefile" => $sd->getPredictableDeviceFile(),
				"canonicaldevicefile" => $sd->getCanonicalDeviceFile(),
				"root" => $objectv['mountpoint'] === "/",
				"description" => $sd->getDescription()
			];
		};
		return $result;
	}
}
