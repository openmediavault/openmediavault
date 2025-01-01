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
namespace OMV\System\Storage;

/**
 * Implements the interface for S.M.A.R.T. AT Attachment (ATA) devices.
 * @see https://www.smartmontools.org/browser/trunk/smartmontools/smartctl.8.in
 * @ingroup api
 */
trait SmartTrait {
	/**
	 * Get the device file, e.g. /dev/sda.
	 * @return string Returns the device file.
	 */
	abstract public function getDeviceFile();

	/**
	 * See \OMV\System\Storage\SmartInterface interface definition.
	 */
	public function getSmartDeviceType() {
		return "";
	}

	/**
	 * See \OMV\System\Storage\SmartInterface interface definition.
	 */
	public function getSmartInformation() {
		return new SmartInformation($this);
	}

	/**
	 * See \OMV\System\Storage\SmartInterface interface definition.
	 */
	public function executeSmartSelfTest($test) {
		$testMap = [
			"s" => "short",
			"l" => "long",
			"c" => "conveyance",
			"o" => "offline"
		];
		$test = mb_strtolower($test);
		if (TRUE === array_key_exists($test, $testMap))
			$test = $testMap[$test];
		$cmdArgs = [];
		$cmdArgs[] = "--xall";
		$cmdArgs[] = sprintf("--test=%s", $test);
		if (!empty($this->getSmartDeviceType())) {
			$cmdArgs[] = sprintf("--device=%s", $this->getSmartDeviceType());
		}
		$cmdArgs[] = $this->getDeviceFile();
		$cmd = new \OMV\System\Process("smartctl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}
}
