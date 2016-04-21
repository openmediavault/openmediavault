<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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

require_once("openmediavault/functions.inc");

/**
 * This class provides a simple interface to handle a LVM logical volume.
 * @ingroup api
 */
class StorageDeviceLvm extends StorageDeviceDM {
	protected $uuid = "";
	protected $attr = "";
	protected $kernelMajor = "";
	protected $kernelMinor = "";
	protected $lvName = "";
	protected $vgName = "";
	private $dataCached = FALSE;

	/**
	 * Get the logical volume detailed information.
	 * @private
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	private function getData() {
		if (FALSE !== $this->dataCached)
			return;

		// Parse command output:
		// VOSP6y-Lw75-pbTs-3v8A-6Qek-XBKJ-Md2G1q|vltest|test|12582912B
		$cmdArgs = [];
		$cmdArgs[] = "--noheadings";
		$cmdArgs[] = "--separator '|'";
		$cmdArgs[] = "-C";
		$cmdArgs[] = "-o lv_uuid,lv_name,vg_name,lv_size,lv_attr,".
		  "lv_kernel_major,lv_kernel_minor";
		$cmdArgs[] = "--unit b";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("lvdisplay", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);

		$output = explode("|", trim($output[0]));

		$this->uuid = $output[0];
		$this->lvName = $output[1];
		$this->vgName = $output[2];
		$this->size = substr($output[3], 0, -1);
		$this->attr = $output[4];
		$this->kernelMajor = intval($output[5]);
		$this->kernelMinor = intval($output[6]);

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return None.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		$this->getData();
	}

	/**
	 * Checks if the logical volume exists.
	 * @return TRUE if the logical volume exists, otherwise FALSE.
	 */
	public function exists() {
		try {
			$this->getData();
		} catch(Exception $e) {
			return FALSE;
		}
		return !empty($this->uuid);
	}

	/**
	 * Get the name of the volume group.
	 * @return The volume group name, FALSE on failure.
	 */
	public function getVGName() {
		$this->getData();
		return $this->vgName;
	}

	/**
	 * Get the name of the logical volume.
	 * @return The logical volume name, e.g. lvol0, or FALSE on failure.
	 */
	public function getName() {
		$this->getData();
		return $this->lvName;
	}

	/**
	 * Get the path of the logical volume.
	 * @return The logical volume path, e.g. /dev/vg0/lvol0.
	 */
	public function getPath() {
		$this->getData();
		return build_path(DIRECTORY_SEPARATOR, "/dev", $this->getVGName(),
		  $this->getName());
	}

	/**
	 * Get the size of the logical volume in bytes.
	 * @return The size of the logical volume in bytes as string.
	 */
	public function getSize() {
		$this->getData();
		return parent::getSize();
	}

	/**
	 * Get the UUID of the array.
	 * @return The UUID of the array.
	 */
	public function getUuid() {
		$this->getData();
		return $this->uuid;
	}

	/**
	 * Get the description of the logical volume.
	 * @return The logical volume description.
	 */
	public function getDescription() {
		$this->getData();
		return sprintf(gettext("LVM logical volume %s [%s, %s]"),
		  $this->getName(), $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}

	/**
	 * Create the logical volume.
	 * @param name The name for the new logical volume.
	 * @param size Gives the size of bytes to allocate for the new
	 *   logical volume.
	 * @param vgName The name of the volume group where to create the
	 *   logical volume.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function create($name, $size, $vgName) {
		$cmdArgs = [];
		$cmdArgs[] = sprintf("--name %s", escapeshellarg($name));
		$cmdArgs[] = sprintf("--size %sK", binary_convert($size, "B", "KiB"));
		$cmdArgs[] = escapeshellarg($vgName);
		$cmd = new \OMV\System\Process("lvcreate", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Remove the logical volume.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function remove() {
		$cmdArgs = [];
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("lvremove", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Rename the logical volume.
	 * @param name The new logical volume path/name.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function rename($name) {
		$this->getData();
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmdArgs[] = escapeshellarg($name);
		$cmd = new \OMV\System\Process("lvrename", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Extend the logical volume.
	 * @param size Gives the size of bytes to extend the logical volume.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function extend($size) {
		// Convert size to KiB to ensure it is a multiple of 512.
		$cmdArgs = [];
		$cmdArgs[] = sprintf("--size %sK", binary_convert($size, "B", "KiB"));
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("lvextend", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Reduce the logical volume.
	 * @param size Gives the size of bytes to extend the logical volume.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function reduce($size) {
		// Convert size to KiB to ensure it is a multiple of 512.
		$cmdArgs = [];
		$cmdArgs[] = sprintf("--size %sK", binary_convert($size, "B", "KiB"));
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("lvreduce", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Enumerate LVM logical volumes.
	 * @return A list of LVM logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 * @throw \OMV\ExecException
	 */
	public static function enumerate() {
		$cmdArgs = [];
		$cmdArgs[] = "--noheadings";
		$cmdArgs[] = "--separator '|'";
		$cmdArgs[] = "-C";
		$cmdArgs[] = "-o vg_name,lv_name";
		$cmd = new \OMV\System\Process("lvdisplay", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);
		$result = [];
		foreach ($output as $outputk => $outputv) {
			$outputv = explode("|", trim($outputv));
			// Replace '-' with '--' in volume group name.
			$result[] = sprintf("/dev/mapper/%s-%s",
			  str_replace("-", "--", $outputv[0]),
			  str_replace("-", "--", $outputv[1]));
			// Alternative device path: /dev/<vg_name>/<lv_name>
			//$list[] = sprintf("/dev/%s/%s", $outputv[0], $outputv[1]);
		}
		return $result;
	}
}
