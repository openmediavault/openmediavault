<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/globals.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");

/**
 * Class to handle a LVM physical volume.
 */
class OMVLvmPhysicalVolume extends OMVObject {
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
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if($this->dataCached !== FALSE)
			return TRUE;

		// Parse command output:
		// gHikbD-106X-xVOQ-7wtr-Q6PO-Pn3Q-UTmNls|vg0|8585740288B|8585740288B|0B
		$cmd = sprintf("export LANG=C; pvdisplay --noheadings ".
		  "--separator '|' -C -o pv_uuid,pv_size,pv_free,pv_used,vg_uuid,".
		  "vg_name,pv_pe_alloc_count --unit b %s", $this->getDeviceFile());
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}

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

		return TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the physical volume exists.
	 * @return TRUE if the physical volume exists, otherwise FALSE.
	 */
	public function exists() {
		if($this->getData() === FALSE)
			return FALSE;
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
	 * @return The UUID of the physical volume, FALSE on failure.
	 */
	public function getUuid() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * Get the size of the physical volume in bytes.
	 * @return The size of the physical volume in bytes as string,
	 * FALSE on failure.
	 */
	public function getSize() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->size;
	}

	/**
	 * Get the total amount of unallocated space.
	 * @return The total amount of unallocated space in bytes as string or
	 * FALSE on failure.
	 */
	public function getFree() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->free;
	}

	/**
	 * Get the total amount of allocated space.
	 * @return The total amount of allocated space in bytes as string or
	 * FALSE on failure.
	 */
	public function getUsed() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->used;
	}

	/**
	 * Get total number of allocated physical extents.
	 * @return The total number of allocated physical extents or FALSE
	 * on failure.
	 */
	public function getAllocatedPhysicalExtents() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->allocatedPE;
	}

	/**
	 * Get the name of the volume group the physical volume is assigned to.
	 * @return The volume group name or FALSE on failure.
	 */
	public function getVGName() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->vgName;
	}

	/**
	 * Get the UUID of the volume group the physical volume is assigned to.
	 * @return The volume group UUID or FALSE on failure.
	 */
	public function getVGUuid() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->vgName;
	}

	/**
	 * Get the description of the physical volume.
	 * @return The physical volume description or FALSE on failure.
	 */
	public function getDescription() {
		if($this->getData() === FALSE)
			return FALSE;
		return sprintf("LVM physical volume [%s, %s]", $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}

	/**
	 * Remove the physical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function remove() {
		$cmd = sprintf("export LANG=C; pvremove --yes --force %s 2>&1",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}
}

/**
 * Class to handle LVM volume groups.
 */
class OMVLvmVolumeGroups extends OMVObject {
	/**
	 * Get a list of volume groups.
	 * @deprecated
	 * @return A list of volume groups or FALSE on failure.
	 * Example: array(
	 *   0 => vg0
	 *   1 => music
	 *   2 => data
	 * )
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate LVM volume groups.
	 * @return A list of LVM volume groups or FALSE on failure.
	 * Example: array(
	 *   0 => vg0
	 *   1 => music
	 *   2 => data
	 * )
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; vgdisplay --noheadings -C -o vg_name ".
		  "2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$list = array();
		// Parse command output:
		//   vg0
		foreach ($output as $outputk => $outputv) {
			$list[] = trim($outputv);
		}
		return $list;
	}
}

/**
 * Class to handle a LVM volume group.
 */
class OMVLvmVolumeGroup extends OMVObject {
	protected $uuid = "";
	protected $name = "";
	protected $size = "";
	protected $free = "";
	protected $pvCount = 0;
	protected $lvCount = 0;
	protected $pvName = array();
	protected $lvName = array();
	protected $extentSize = "";
	protected $extentCount = "";
	protected $freeCount = "";
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param name The name of the volume group, e.g. 'vg02' or '/dev/vg02'.
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Get the volume group detailed informations.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if($this->dataCached !== FALSE)
			return TRUE;

		// Parse command output:
		// gHikbD-106X-xVOQ-7wtr-Q6PO-Pn3Q-UTmNls|vg0|8585740288B|8552185856B|1|2
		$cmd = sprintf("export LANG=C; vgdisplay --noheadings ".
		  "--separator '|' -C -o vg_uuid,vg_name,vg_size,vg_free,pv_count,".
		  "lv_count,vg_extent_size,vg_extent_count,vg_free_count --unit b %s",
		  $this->getName());
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;

		$output = explode("|", trim($output[0]));

		$this->uuid = $output[0];
		$this->name = $output[1];
		$this->size = substr($output[2], 0, -1);
		$this->free = substr($output[3], 0, -1);
		$this->pvCount = intval($output[4]);
		$this->lvCount = intval($output[5]);
		$this->extentSize = substr($output[6], 0, -1);
		$this->extentCount = $output[7];
		$this->freeCount = $output[8];

		// Get the names of the physical volumes.
		$this->pvName = array();
		if($this->pvCount > 0) {
			unset($output);
			$cmd = sprintf("export LANG=C; vgdisplay --noheadings ".
			  "--separator '|' -C -o pv_name %s 2>/dev/null",
			  escapeshellarg($this->getName()));
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0) {
				$this->setLastError($output);
				return FALSE;
			}

			foreach($output as $outputk => $outputv) {
				$this->pvName[] = trim($outputv);
			}
		}

		// Get the names of the logical volumes.
		$this->lvName = array();
		if($this->lvCount > 0) {
			unset($output);
			$cmd = sprintf("export LANG=C; vgdisplay --noheadings ".
			  "--separator '|' -C -o lv_name %s 2>/dev/null",
			  escapeshellarg($this->getName()));
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0) {
				$this->setLastError($output);
				return FALSE;
			}

			foreach($output as $outputk => $outputv) {
				$this->lvName[] = trim($outputv);
			}
		}

		// Set flag to mark informations has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the volume group exists.
	 * @return TRUE if the volume group exists, otherwise FALSE.
	 */
	public function exists() {
		if($this->getData() === FALSE)
			return FALSE;
		return !empty($this->uuid);
	}

	/**
	 * Get the name of the volume group.
	 * @return The volume group name.
	 */
	public function getName() {
		return $this->name;
	}

	/**
	 * Get the device path of the volume group, e.g '/dev/vg02'.
	 * @return The device path, otherwise FALSE.
	 */
	public function getDeviceFile() {
		if($this->getData() === FALSE)
			return FALSE;
		return sprintf("/dev/%s", $this->name);
	}

	/**
	 * Get the UUID of the volume group.
	 * @return The UUID of the volume group, FALSE on failure.
	 */
	public function getUuid() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * Get the size of the volume group in bytes.
	 * @return The size of the volume group in bytes as string or
	 * FALSE on failure.
	 */
	public function getSize() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->size;
	}

	/**
	 * Get the total amount of unallocated space.
	 * @return The total amount of unallocated space in bytes as string
	 * or FALSE on failure.
	 */
	public function getFree() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->free;
	}

	/**
	 * Get the number of physical volumes assigned to this volume group.
	 * @return The number of physical volumes or FALSE on failure.
	 */
	public function getPVCount() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->pvCount;
	}

	/**
	 * Get the devicefiles of the physical volumes assigned to this volume
	 * group.
	 * @return An array of the physical volume devicefiles or FALSE on failure.
	 */
	public function getPVName() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->pvName;
	}

	/**
	 * Get the number of logical volumes assigned to this volume group.
	 * @return The number of logical volumes or FALSE on failure.
	 */
	public function getLVCount() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->lvCount;
	}

	/**
	 * Get the names of the logical volumes assigned to this volume group.
	 * @return An array of logical volume names or FALSE on failure.
	 */
	public function getLVName() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->lvName;
	}

	/**
	 * Get the size of physical extents in bytes.
	 * @return The size of physical extents in bytes as string or FALSE
	 *   on failure.
	 */
	public function getExtentSize() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->extentSize;
	}

	/**
	 * Get the total number of physical extents.
	 * @return the total number of physical extents or FALSE on failure.
	 */
	public function getNumExtents() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->extentCount;
	}

	/**
	 * Get the total number of unallocated physical extents.
	 * @return The total number of unallocated physical extents as string
	 *   or FALSE on failure.
	 */
	public function getNumFreeExtents() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->freeCount;
	}

	/**
	 * Get the description of the volume group.
	 * @return The volume group description or FALSE on failure.
	 */
	public function getDescription() {
		if($this->getData() === FALSE)
			return FALSE;
		return sprintf("LVM volume group [%s, %s, %s free]",
		  $this->getDeviceFile(), binary_format($this->getSize()),
		  binary_format($this->getFree()));
	}

	/**
	 * Remove the volume group.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function remove() {
		$cmd = sprintf("export LANG=C; vgremove --force %s 2>&1",
		  escapeshellarg($this->getName()));
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}
}
?>
