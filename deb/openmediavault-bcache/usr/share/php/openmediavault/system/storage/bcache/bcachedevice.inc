<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Michael Myrcik <michael.myrcik@web.de>
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
namespace OMV\System\Storage\Bcache;

/**
 * This class provides a simple interface to handle a Bcache devices.
 */
class BcacheDevice extends \OMV\System\Storage\StorageDevice {
	private $csetUuid = "";
	private $superBlock = [];
	private $dataCached = FALSE;

	/**
	 * Get the software bcache device detailed information.
	 * @private
	 * @return void
	 * @throw \OMV\ExecException
	 */
	private function getData() {
		if ($this->dataCached !== FALSE) {
			return;
		}

		$cmdArgs = [];
		$cmdArgs[] = "-f";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("bcache-super-show", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$this->superBlock = $output;
		$attributes = [];
		foreach ($output as $outputk => $outputv) {
			$keyValue = preg_split('/\s+/', $outputv);
			if (count($keyValue) != 2) {
				continue;
			}
			$key = mb_strtolower(trim($keyValue[0]));
			$value = trim($keyValue[1]);
			$attributes[$key] = $value;
		}
		$this->csetUuid = $attributes['cset.uuid'];

		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Get the device CSET-UUID.
	 * @return The device CSET-UUID.
	 */
	public function getCsetUuid() {
		$this->getData();
		return $this->csetUuid;
	}

	/**
	 * Is bcache devices known by the kernel.
	 * @return boolean
	 * @throw \OMV\ExecException
	 */
	public function isRegistered() {
		$path = "/sys/block/{$this->getDeviceName(TRUE)}/bcache";
		return file_exists($path);
	}

	/**
	 * Get super block of the bcache device.
	 * @return The super block of the bcache device.
	 */
	public function getSuperBlock() {
		$this->getData();
		return implode("\n", $this->superBlock);
	}

	/**
	 * Enumerate bcache devices.
	 * @return A list of bcache devices.
	 * Example: array(
	 *   0 => /dev/sda
	 *   1 => /dev/sdb
	 * )
	 * @throw \OMV\ExecException
	 */
	public static function enumerateBcache() {
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$fsbMngr->assertBackendExistsByType('bcache');
		$objects = $fsbMngr->enumerate();
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			// Skip none bcache filesystem
			if ($objectv['type'] !== 'bcache') {
				continue;
			}
			$devicefile = $objectv['devicefile'];
			// Prepare result object.
			$cmdLine = sprintf("bcache-super-show -f %s ".
				"| sed -n 's/^sb.*\(cache\|backing\).*/\\1/p'", $devicefile);
			$cmd = new \OMV\System\Process($cmdLine);
			$cmd->setRedirect2to1();
			$output = [];
			$cmd->execute($output, $exitStatus);
			if (0 !== $exitStatus) {
				continue;
			}
			$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
				$devicefile);
			$result[] = [
				"uuid" => $objectv['uuid'],
				"devicefile" => $devicefile,
				"label" => $objectv['label'],
				"size" => $sd->getSize(),
				"bcachetype" => $output[0],
				"description" => $sd->getDescription()
			];
		}
		return $result;
	}

	/**
	 * Register as bcache device.
	 * Registering the backing device makes the bcache device show up in /dev.
	 * @param string deviceFile Specifies the device file, e.g.
	 * @return boolean
	 * @throw \OMV\ExecException
	 */
	public static function register($deviceFile) {
		$filename = "/sys/fs/bcache/register";
		$cmdLine = sprintf("echo %s > %s", $deviceFile, $filename);
		$cmd = new \OMV\System\Process($cmdLine);
		$cmd->setRedirect2to1();
		$cmd->execute($output, $exitStatus);
		return $exitStatus === 0;
	}
}
