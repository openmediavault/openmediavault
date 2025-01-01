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
namespace OMV\System\Filesystem\Backend;

require_once("openmediavault/functions.inc");

/**
 * Helper class that implements functions regarding file systems.
 * @ingroup api
 */
class Manager implements \IteratorAggregate {
	private $map = [];

	/**
	 * Returns a manager singleton.
	 * @return The manager object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new Manager();
		return $instance;
	}

	public function getIterator() {
		return new \ArrayIterator($this->map);
	}

	/**
	 * Register a filesystem backend.
	 * @param backend The filesystem backend object to register.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final public function registerBackend(BackendAbstract $backend) {
		if (!isset($backend))
			return FALSE;
		$type = mb_strtolower($backend->getType());
		// Check if the filesystem backend already exists.
		if (NULL !== ($registeredBackend = $this->getBackendByType($type))) {
			throw new \OMV\Exception("The file system backend (type=%s, ".
			  "class=%s) is already registered (class=%s).", $type,
			  get_class($backend), get_class($registeredBackend));
		}
		$this->map[$type] = $backend;
		ksort($this->map);
		return TRUE;
	}

	/**
	 * Unregister a filesystem backend.
	 * @param type Specifies the filesystem type, e.g. 'ext3', 'vfat', ...
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final public function unregisterBackend($type) {
		$type = mb_strtolower($type);
		if (NULL === $this->getBackendByType($type))
			return FALSE;
		unset($this->map[$type]);
		ksort($this->map);
		return TRUE;
	}

	/**
	 * Get the backend of the specified filesystem type.
	 * @param type Specifies the filesystem type, e.g. 'ext3', 'vfat', ...
	 * @return The backend that implements the given filesystem, otherwise
	 *   NULL.
	 */
	final public function getBackendByType($type) {
		$type = mb_strtolower($type);
		if (!array_key_exists($type, $this->map))
			return NULL;
		return $this->map[$type];
	}

	/**
	 * Assert that the backend for the specified filesystem type exists.
	 * @param type Specifies the filesystem type, e.g. 'ext3', 'vfat', ...
	 * @throw \OMV\AssertException
	 */
	final public function assertBackendExistsByType($type) {
		$backend = $this->getBackendByType($type);
		if (is_null($backend)) {
			throw new \OMV\AssertException(
			  "No file system backend exists for type '%s'.",
			  $type);
		}
	}

	/**
	 * Get the backend of the specified filesystem identifier.
	 * @param id The UUID, label or device path of the filesystem, e.g.
	 *   <ul>
	 *   \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li 7A48-BA97 (FAT/exFAT)
	 *   \li 2ED43920D438EC29 (NTFS)
	 *   \li 2015-01-13-21-48-46-00 (ISO9660)
	 *   \li /dev/sde1
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6-part1
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0-part2
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   \li /dev/cciss/c0d0p2
	 *   \li tank/multimedia/movies
	 *   \li BigDataRAID (unescaped label)
	 *   </ul>
	 * @return The backend that implements the given filesystem, otherwise
	 *   NULL.
	 */
	final public function getBackendById($id) {
		$result = NULL;
		$enums = $this->enumerate();
		foreach ($enums as $enumk => $enumv) {
			$found = FALSE;
			if (is_devicefile($id)) {
				// Compare the device file name. Use the canonical device
				// files for an additional check.
				if ($id == $enumv['devicefile'])
					$found = TRUE;
				else if (realpath($id) == realpath($enumv['devicefile']))
					$found = TRUE;
			} else if (is_fs_uuid($id)) {
				// Compare the filesystem UUID.
				$found = ($id == $enumv['uuid']);
			} else {
				$found = ($id == $enumv['label']);
			}
			if (TRUE === $found) {
				$result = $this->getBackendByType($enumv['type']);
				break;
			}
		}
		return $result;
	}

	/**
	 * Assert that the backend for the specified filesystem exists.
	 * @param id The UUID, label or device path of the filesystem, e.g.
	 *   <ul>
	 *   \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li 7A48-BA97 (FAT)
	 *   \li 2ED43920D438EC29 (NTFS)
	 *   \li 2015-01-13-21-48-46-00 (ISO9660)
	 *   \li /dev/sde1
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6-part1
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0-part2
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   \li /dev/cciss/c0d0p2
	 *   \li tank/multimedia/movies
	 *   \li BigDataRAID (unescaped label)
	 *   </ul>
	 * @throw \OMV\AssertException
	 */
	final public function assertBackendExistsById($id) {
		$backend = $this->getBackendById($id);
		if (is_null($backend)) {
			throw new \OMV\AssertException(
			  "No file system backend exists for '%s'.",
			  $id);
		}
	}

	/**
	 * Check whether the given file system type is supported.
	 * @param type The file system type, e.g. 'ext4', 'iso9660', ...
	 * @return TRUE if the file system is supported, otherwise FALSE.
	 */
	final public function isSupported($type) {
		if (NULL == ($backend = $this->getBackendByType($type)))
			return FALSE;
		return TRUE;
	}

	/**
	 * Enumerate all available/detected file systems.
	 * @return An array of objects with the fields \em devicefile, \em uuid,
	 *   \em label (unescaped) and \em type.
	 * @throw \OMV\ExecException
	 * @throw \UnexpectedValueException
	 */
	final public function enumerate() {
		$result = [];
		$blkidEnumsByType = [];
		$output = [];
		// Get all file systems that are identified by the block device
		// identification library.
		// !!! Note !!!
		// File systems that are identified by the block device identification
		// library are processed in one step to increase performance. Otherwise
		// each backend must implement the \ref enumerate method and call the
		// command 'blkid -t TYPE=<xxx> -o full'.
		$cmd = new \OMV\System\Process("blkid", "-o full");
		$cmd->setRedirect2to1();
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		if (!in_array($exitStatus, [0, 2])) {
			throw new \OMV\ExecException($cmd, $output, $exitStatus);
		}
		foreach ($output as $outputk => $outputv) {
			// Parse command output:
			// /dev/sdb: UUID="e6a0f61e-d969-ffcb-d4c5-06e7711285a3" LABEL="dhcppc4:0" TYPE="linux_raid_member"
			// /dev/sdg1: LABEL="test" UUID="d19bcea0-a323-4aea-9791-c6578180c129" TYPE="jfs"
			// /dev/sda5: UUID="902775c7-8250-4630-81eb-2602ac65e136" TYPE="swap"
			// /dev/sda1: LABEL="Ubuntu" UUID="4aa77ba3-beb6-45dc-b2c7-e77cffd6c6fd" TYPE="ext4"
			// /dev/sdb1: UUID="A27E16CD7E1699DD" TYPE="ntfs"
			// /dev/sdc1: UUID="1a03b79f-d347-4a24-8ae2-62f1aa9a4554" TYPE="ext4"
			// /dev/sdd1: UUID="cb8f7844-362f-4151-8e9e-3888aab723c2" TYPE="ext4"
			// /dev/sda3: UUID="fdca09a8-cbc8-432f-9cba-ce033cb59139" TYPE="ext4" LABEL="Debian"
			// /dev/sr0: LABEL="Debian 7.5.0 amd64 1" TYPE="iso9660"
			// /dev/sr0: UUID="2015-04-25-12-52-13-00" LABEL="Debian 8.0.0 amd64 1" TYPE="iso9660" PTUUID="5e97595e" PTTYPE="dos"
			// /dev/sda1: LABEL="Music" UUID="54AF-0252" TYPE="exfat"
			// /dev/sdk: LABEL="TestFS" UUID="b7adba40-b980-4067-b33c-dd5c8ee17044" UUID_SUB="7b19094b-e7df-4a3e-a3db-57e2d4c548b3" TYPE="btrfs"
			// /dev/sdl: LABEL="TestFS" UUID="b7adba40-b980-4067-b33c-dd5c8ee17044" UUID_SUB="ba1e2b16-5022-4cbd-ad35-6d609e9560e9" TYPE="btrfs"
			$regex = '/^(\S+):\s*(.+)$/i';
			unset($matches);
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			// Set default values and extract key/value pairs.
			$data = [
				"devicefile" => $matches[1],
				"uuid" => "",
				"label" => "",
				"type" => ""
			];
			$parts = preg_split("/(\S+=\\\"[^\\\"]+\\\")|[\s]+/", $matches[2],
			  -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
			foreach ($parts as $partk => $partv) {
				$keyValue = explode("=", $partv);
				if (count($keyValue) != 2)
					continue;
				$data[mb_strtolower($keyValue[0])] = substr(
					$keyValue[1], 1, -1);
			}
			// Append the found filesystem to the list for further processing.
			if (!array_key_exists($data['type'], $blkidEnumsByType))
				$blkidEnumsByType[$data['type']] = [];
			$blkidEnumsByType[$data['type']][] = $data;
		}
		// Now process all filesystems that are supported.
		foreach ($this->map as $backendk => $backendv) {
			$enums = [];
			if (TRUE === $backendv->isBlkidEnumerated()) {
				// Get the filesystem type handled by this backend, e.g.
				// 'vfat', 'ntfs' or 'ext4'.
				$type = $backendv->getType();
				// Check if the 'blkid' command in the previous step has
				// detected some filesystems of this type.
				if (!array_key_exists($type, $blkidEnumsByType))
					continue;
				// Get the details of the filesystems that are detected by
				// 'blkid'. This is an array of objects with (at least) the
				// fields \em devicefile, \em uuid, \em label and \em type.
				$blkidEnums = $blkidEnumsByType[$type];
				// Finally let the backend process the list of detected
				// filesystems.
				$enums = $backendv->enumerateByBlkid($blkidEnums);
			} else {
				// Ask the backend for filesystem candidates.
				$enums = $backendv->enumerate();
			}
			// Validate the filesystems to ensure that the associative
			// array/dictionary contains at least the required fields.
			foreach ($enums as $enumk => $enumv) {
				$requiredKeys = ['devicefile', 'uuid', 'label', 'type'];
				if (!array_keys_exists($requiredKeys, $enumv)) {
					throw new \UnexpectedValueException(sprintf(
						"The array does not contain the required keys %s.",
						implode(", ", $requiredKeys)));
				}
				$result[] = $enumv;
			}
		}
		return $result;
	}

	/**
	 * Dump all registered RPC services.
	 */
	final public function dump() {
		print("Registered file system backends:\n");
		foreach ($this->map as $backendk => $backendv)
			printf("  %s\n", $backendv->getType());
	}
}
