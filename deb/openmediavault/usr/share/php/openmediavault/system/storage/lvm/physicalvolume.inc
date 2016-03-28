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
namespace OMV\System\Storage\Lvm;

/**
 * Class to handle a LVM physical volume.
 */
class PhysicalVolume {
	protected $deviceFile = "";
	protected $uuid = "";
	protected $size = "";
	protected $free = "";
	protected $used = "";
	protected $vgUuid = "";
	protected $vgName = "";
	protected $allocatedPE = 0;
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g. /dev/sdb.
	 */
	public function __construct($deviceFile) {
		$this->deviceFile = $deviceFile;
	}

	/**
	 * Get the physical volume detailed informations.
	 * @private
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	private function getData() {
		if($this->dataCached !== FALSE)
			return;

		// Parse command output:
		// gHikbD-106X-xVOQ-7wtr-Q6PO-Pn3Q-UTmNls|vg0|8585740288B|8585740288B|0B
		$cmdArgs = array();
		$cmdArgs[] = "--noheadings";
		$cmdArgs[] = "--separator '|'";
		$cmdArgs[] = "-C";
		$cmdArgs[] = "-o pv_uuid,pv_size,pv_free,pv_used,vg_uuid,vg_name,".
		  "pv_pe_alloc_count";
		$cmdArgs[] = "--unit b";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("pvdisplay", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);

		$output = explode("|", trim($output[0]));

		$this->uuid = $output[0];
		$this->size = substr($output[1], 0, -1);
		$this->free = substr($output[2], 0, -1);
		$this->used = substr($output[3], 0, -1);
		$this->vgUuid = $output[4];
		$this->vgName = $output[5];
		$this->allocatedPE = intval($output[6]);

		// Set flag to mark informations has been successfully read.
		$this->dataCached = TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return None.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		$this->getData();
	}

	/**
	 * Checks if the physical volume exists.
	 * @return TRUE if the physical volume exists, otherwise FALSE.
	 */
	public function exists() {
		try {
			$this->getData();
		} catch(\Exception $e) {
			return FALSE;
		}
		return !empty($this->uuid);
	}

	/**
	 * Get the device path, e.g. /dev/vg0/lvol0.
	 * @return The device path.
	 */
	public function getDeviceFile() {
		return $this->deviceFile;
	}

	/**
	 * Get the UUID of the physical volume.
	 * @return The UUID of the physical volume.
	 */
	public function getUuid() {
		$this->getData();
		return $this->uuid;
	}

	/**
	 * Get the size of the physical volume in bytes.
	 * @return The size of the physical volume in bytes as string.
	 */
	public function getSize() {
		$this->getData();
		return $this->size;
	}

	/**
	 * Get the total amount of unallocated space.
	 * @return The total amount of unallocated space in bytes as string.
	 */
	public function getFree() {
		$this->getData();
		return $this->free;
	}

	/**
	 * Get the total amount of allocated space.
	 * @return The total amount of allocated space in bytes as string.
	 */
	public function getUsed() {
		$this->getData();
		return $this->used;
	}

	/**
	 * Get total number of allocated physical extents.
	 * @return The total number of allocated physical extents.
	 */
	public function getAllocatedPhysicalExtents() {
		$this->getData();
		return $this->allocatedPE;
	}

	/**
	 * Get the name of the volume group the physical volume is assigned to.
	 * @return The volume group name.
	 */
	public function getVGName() {
		$this->getData();
		return $this->vgName;
	}

	/**
	 * Get the UUID of the volume group the physical volume is assigned to.
	 * @return The volume group UUID.
	 */
	public function getVGUuid() {
		$this->getData();
		return $this->vgName;
	}

	/**
	 * Get the description of the physical volume.
	 * @return The physical volume description.
	 */
	public function getDescription() {
		$this->getData();
		return sprintf("LVM physical volume [%s, %s]", $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}

	/**
	 * Create the physical volume.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function create() {
		$cmdArgs = array();
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("pvcreate", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Remove the physical volume.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function remove() {
		$cmdArgs = array();
		$cmdArgs[] = "--yes";
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("pvremove", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Resize the physical volume.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function resize() {
		$cmdArgs = array();
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("pvresize", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Enumerate LVM physical volumes.
	 * @return A list of LVM physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 * @throw \OMV\ExecException
	 */
	public static function enumerate() {
		$cmdArgs = array();
		$cmdArgs[] = "--noheadings";
		$cmdArgs[] = "--separator '|'";
		$cmdArgs[] = "-C";
		$cmdArgs[] = "-o pv_name";
		$cmd = new \OMV\System\Process("pvdisplay", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);
		// Parse command output:
		//   /dev/sdc
		//   /dev/sdd
		//   unknown device
		$result = array();
		foreach($output as $outputk => $outputv) {
			$deviceFile = trim($outputv);
			if (!is_devicefile($deviceFile))
				continue;
			$result[] = $deviceFile;
		}
		return $result;
	}
}
