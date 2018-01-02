<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2018 Volker Theile
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
namespace OMV\System\Storage;

/**
 * An abstract class which implements the interface for S.M.A.R.T.
 * AT Attachment (ATA) devices.
 * @ingroup api
 */
abstract class SmartStorageDeviceAbstract extends StorageDevice
  implements ISmartStorageDevice {
	private $smartDeviceType = "";

	/**
	 * Set the device type required by the smartctl utility program.
	 * @return Returns the device type.
	 */
	protected function setSmartDeviceType($type) {
		$this->smartDeviceType = $type;
		return $this->smartDeviceType;
	}

	/**
	 * See interface definition.
	 */
	public function getSmartDeviceType() {
		return $this->smartDeviceType;
	}

	/**
	 * See interface definition.
	 */
	public function getSmartInformation() {
		$smartInfo = new SmartInformation($this->getDeviceFile(),
		  $this->getSmartDeviceType(), $this);
		if (is_null($smartInfo)) {
			throw new \OMV\Exception(
			  "Failed to get S.M.A.R.T. interface for device '%s'.",
			  $this->getDeviceFile());
		}
		return $smartInfo;
	}

	/**
	 * See interface definition.
	 */
	public function executeSmartSelfTest($test) {
		$testMap = [
			"s" => "short",
			"l" => "long",
			"c" => "conveyance",
			"o" => "offline"
		];
		$test = strtolower($test);
		if (TRUE === array_key_exists($test, $testMap))
			$test = $testMap[$test];
		$cmdArgs = [];
		$cmdArgs[] = "--xall";
		$cmdArgs[] = sprintf("--test=%s", $test);
		if (!empty($this->getSmartDeviceType()))
			$cmdArgs[] = sprintf("--device=%s", $this->getSmartDeviceType());
		$cmdArgs[] = $this->getDeviceFile();
		$cmd = new \OMV\System\Process("smartctl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}
}
