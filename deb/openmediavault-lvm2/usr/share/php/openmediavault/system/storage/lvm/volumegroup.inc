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
namespace OMV\System\Storage\Lvm;

require_once("openmediavault/functions.inc");

/**
 * Class to handle a LVM volume group.
 */
class VolumeGroup {
	protected $uuid = "";
	protected $name = "";
	protected $size = "";
	protected $free = "";
	protected $pvCount = 0;
	protected $lvCount = 0;
	protected $pvName = [];
	protected $lvName = [];
	protected $extentSize = "";
	protected $extentCount = "";
	protected $freeCount = "";
	protected $attr = "";
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
	 * @return void
	 * @throw \OMV\ExecException
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return;

		// Parse command output:
		// gHikbD-106X-xVOQ-7wtr-Q6PO-Pn3Q-UTmNls|vg0|8585740288B|8552185856B|1|2
		$cmdArgs = [];
		$cmdArgs[] = "--noheadings";
		$cmdArgs[] = "--separator '|'";
		$cmdArgs[] = "-C";
		$cmdArgs[] = "-o vg_uuid,vg_name,vg_size,vg_free,pv_count,lv_count,".
		  "vg_extent_size,vg_extent_count,vg_free_count,vg_attr";
		$cmdArgs[] = "--unit b";
		$cmdArgs[] = escapeshellarg($this->getName());
		$cmd = new \OMV\System\Process("vgdisplay", $cmdArgs);
		$cmd->setRedirect2toFile("/dev/null");
		$cmd->execute($output);

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
		$this->attr = $output[9];

		// Get the names of the physical volumes.
		$this->pvName = [];
		if ($this->pvCount > 0) {
			unset($output);
			$cmdArgs = [];
			$cmdArgs[] = "--noheadings";
			$cmdArgs[] = "--separator '|'";
			$cmdArgs[] = "-C";
			$cmdArgs[] = "-o pv_name";
			$cmdArgs[] = escapeshellarg($this->getName());
			$cmd = new \OMV\System\Process("vgdisplay", $cmdArgs);
			$cmd->setRedirect2toFile("/dev/null");
			$cmd->execute($output);

			foreach ($output as $outputk => $outputv)
				$this->pvName[] = trim($outputv);
		}

		// Get the names of the logical volumes.
		$this->lvName = [];
		if ($this->lvCount > 0) {
			unset($output);
			$cmdArgs = [];
			$cmdArgs[] = "--noheadings";
			$cmdArgs[] = "--separator '|'";
			$cmdArgs[] = "-C";
			$cmdArgs[] = "-o lv_name";
			$cmdArgs[] = escapeshellarg($this->getName());
			$cmd = new \OMV\System\Process("vgdisplay", $cmdArgs);
			$cmd->setRedirect2toFile("/dev/null");
			$cmd->execute($output);

			foreach ($output as $outputk => $outputv)
				$this->lvName[] = trim($outputv);
		}

		// Set flag to mark informations has been successfully read.
		$this->dataCached = TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return void
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		$this->getData();
	}

	/**
	 * Checks if the volume group exists.
	 * @return TRUE if the volume group exists, otherwise FALSE.
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
	 * Assert that the volume group exists.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertExists() {
		if (FALSE === $this->exists()) {
			throw new \OMV\AssertException("Volume group '%s' not found.",
			  $this->getName());
		}
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
	 * @return The device path.
	 */
	public function getDeviceFile() {
		$this->getData();
		return sprintf("/dev/%s", $this->name);
	}

	/**
	 * Get the canonical device file, e.g. <ul>
	 * \li /dev/mapper/vg0-lv0 -> /dev/dm-0
	 * </ul>
	 * @return Returns the canonical device file.
	 */
	public function getCanonicalDeviceFile() {
		return realpath($this->getDeviceFile());
	}

	/**
	 * Get the device file to present in the UI, e.g.:
	 * <ul>
	 * \li /dev/disk/by-id/xxx
	 * \li /dev/disk/by-path/xxx
	 * \li /dev/xxx
	 * </ul>
	 * @return Returns a device file.
	 */
	public function getPreferredDeviceFile() {
		// Get the canonical device file.
		$deviceFile = $this->getCanonicalDeviceFile();
		// Note, if the VG does not contain a logical volume, then there
		// is no device file. In that case simply return the /dev/xxx
		// device file.
		if (!is_block_device($deviceFile))
			$deviceFile = $this->getDeviceFile();
		return $deviceFile;
	}

	/**
	 * Get the UUID of the volume group.
	 * @return The UUID of the volume group.
	 */
	public function getUuid() {
		$this->getData();
		return $this->uuid;
	}

	/**
	 * Get the size of the volume group in bytes.
	 * @return The size of the volume group in bytes as string.
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
	 * Get the number of physical volumes assigned to this volume group.
	 * @return The number of physical volumes.
	 */
	public function getPVCount() {
		$this->getData();
		return $this->pvCount;
	}

	/**
	 * Get the devicefiles of the physical volumes assigned to this volume
	 * group.
	 * @return An array of the physical volume devicefiles.
	 */
	public function getPVName() {
		$this->getData();
		return $this->pvName;
	}

	/**
	 * Get the number of logical volumes assigned to this volume group.
	 * @return The number of logical volumes.
	 */
	public function getLVCount() {
		$this->getData();
		return $this->lvCount;
	}

	/**
	 * Get the names of the logical volumes assigned to this volume group.
	 * @return An array of logical volume names.
	 */
	public function getLVName() {
		$this->getData();
		return $this->lvName;
	}

	/**
	 * Get the size of physical extents in bytes.
	 * @return The size of physical extents in bytes as string.
	 */
	public function getExtentSize() {
		$this->getData();
		return $this->extentSize;
	}

	/**
	 * Get the total number of physical extents.
	 * @return the total number of physical extents.
	 */
	public function getNumExtents() {
		$this->getData();
		return $this->extentCount;
	}

	/**
	 * Get the total number of unallocated physical extents.
	 * @return The total number of unallocated physical extents as string.
	 */
	public function getNumFreeExtents() {
		$this->getData();
		return $this->freeCount;
	}

	/**
	 * Get the volume group attributes.
	 * @see http://www.unixarena.com/2013/08/redhat-linux-lvm-volume-attributes.html
	 * @return Returns an array with the attribtes.
	 */
	public function getAttributes() {
		$this->getData();
		return [
			"access" => [
				"read" => ("r" == substr($this->vgAttr, 0, 1)),
				"write" => ("w" == substr($this->vgAttr, 0, 1))
			],
			"resizeable" => ("z" == substr($this->vgAttr, 1, 1)),
			"exported" => ("x" == substr($this->vgAttr, 2, 1)),
			"partial" => ("p" == substr($this->vgAttr, 3, 1)),
			"allocationpolicy" => [
				"contiguous" => ("c" == substr($this->vgAttr, 4, 1)),
				"cling" => ("l" == substr($this->vgAttr, 4, 1)),
				"normal" => ("n" == substr($this->vgAttr, 4, 1)),
				"anywhere" => ("a" == substr($this->vgAttr, 4, 1)),
				"inherited" => ("i" == substr($this->vgAttr, 4, 1)),
			],
			"cluster" => ("c" == substr($this->vgAttr, 5, 1)),
		];
	}

	/**
	 * Get the description of the volume group.
	 * @return The volume group description.
	 */
	public function getDescription() {
		$this->getData();
		return sprintf("LVM volume group %s [%s, %s, %s free]",
		  $this->getName(), $this->getPreferredDeviceFile(),
		  binary_format($this->getSize()), binary_format($this->getFree()));
	}

	/**
	 * Create the volume group.
	 * @param devices An array containing the physical device paths.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function create($devices) {
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getName());
		$cmdArgs[] = implode(" ", $devices);
		$cmd = new \OMV\System\Process("vgcreate", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Remove the volume group.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function remove() {
		$cmdArgs = [];
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($this->getName());
		$cmd = new \OMV\System\Process("vgremove", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Rename the volume group.
	 * @param name The new volume group path/name.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function rename($name) {
		$this->getData();
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getUuid());
		$cmdArgs[] = escapeshellarg($name);
		$cmd = new \OMV\System\Process("vgrename", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Extend the volume group.
	 * @param devices An array containing the physical device paths to be
	 *   added.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function extend($devices) {
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getName());
		$cmdArgs[] = implode(" ", $devices);
		$cmd = new \OMV\System\Process("vgextend", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Reduce the volume group.
	 * @param devices An array containing the physical device paths to
	 *   be removed.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function reduce($devices) {
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getName());
		$cmdArgs[] = implode(" ", $devices);
		$cmd = new \OMV\System\Process("vgreduce", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Enumerate LVM volume groups.
	 * @return A list of LVM volume groups.
	 * Example: array(
	 *   0 => vg0
	 *   1 => music
	 *   2 => data
	 * )
	 * @throw \OMV\ExecException
	 */
	public static function enumerate() {
		$cmdArgs = [];
		$cmdArgs[] = "--noheadings";
		$cmdArgs[] = "-C";
		$cmdArgs[] = "-o vg_name";
		$cmd = new \OMV\System\Process("vgdisplay", $cmdArgs);
		$cmd->setRedirect2toFile("/dev/null");
		$cmd->execute($output);
		// Parse command output:
		//   vg0
		$result = [];
		foreach ($output as $outputk => $outputv) {
			$result[] = trim($outputv);
		}
		return $result;
	}
}
