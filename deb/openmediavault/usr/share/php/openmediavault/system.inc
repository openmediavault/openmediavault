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
require_once("openmediavault/globals.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");

/**
 * Helper class to get all existing devices of the given storage type.
 * @ingroup api
 */
class OMVStorageDevices extends OMVObject {
	static private $backends = array();

	/**
	 * Register a storage device backend. If the registered backend class is
	 * derived from another backend, then it will be inserted before this.
	 * @param backend The storage device backend object to register.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final static function registerBackend(OMV\System\Storage\Backend\BackendAbstract $backend) {
		if (!isset($backend))
			return FALSE;
		if (FALSE === ($parent = get_parent_class($backend))) {
			self::$backends[] = $backend;
		} else {
			// Try to find the derived class and the position where to
			// insert the storage device backend class to register.
			$offset = FALSE;
			foreach (self::$backends as $backendk => $backendv) {
				if ( ("OMV\System\Storage\Backend\BackendAbstract" === $parent) ||
				  (FALSE === ($backendv instanceof $parent)))
					continue;
				$offset = $backendk;
				break;
			}
			if (FALSE === $offset) {
				self::$backends[] = $backend;
			} else {
				array_splice(self::$backends, $offset, 0, array($backend));
			}
		}
		return TRUE;
	}

	/**
	 * Get the backend of the given device.
	 * @param deviceFile Specifies the device file.
	 * @return The backend that implements the given device file, otherwise
	 *   NULL.
	 */
	final static function getBackend($deviceFile) {
		$result = NULL;
		foreach (self::$backends as $backendk => $backendv) {
			if (FALSE === $backendv->isTypeOf($deviceFile))
				continue;
			$result = $backendv;
			break;
		}
		return $result;
	}

	/**
	 * Enumerate devices matching the given storage device type.
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	public static function enumerate($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerate()))
				return FALSE;
			$result = array_unique(array_merge($result, $devs));
		}
		return $result;
	}

	/**
	 * Enumerate all unused devices. This list contains all devices of the
	 * given type except the devices that are used by other storage devices
	 * (e.g. LVM as physical volume or a member of a Software RAID).
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	final static function enumerateUnused($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		// Append all available storage devices.
		if (FALSE === ($result = self::enumerate($type)))
			return FALSE;
		// Remove used devices.
		foreach (self::$backends as $backendk => $backendv) {
// Always collect and remove all slave devices.
//			if (!($type & $backendv->getType()))
//				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_diff($result, $slaves);
		}
		return $result;
	}

	/**
	 * Enumerate all used devices. The list contains all those devices that
	 * are used by the given storage devices, e.g. all members of a LVM
	 * or Software RAID.
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	final static function enumerateUsed($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_unique(array_merge($result, $slaves));
		}
		return $result;
	}

	/**
	 * Check if the given device is used/consumed by another storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return TRUE if the given device is used/consumed by another storage
	 *   device, otherwise FALSE.
	 */
	final static function isUsed($deviceFile, $type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = FALSE;
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (in_array($deviceFile, $slaves)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the object of the class which implements the given storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return The object of the class implementing the given storage device,
	 *   otherwise NULL.
	 */
	public static function getImpl($deviceFile) {
		if (NULL == ($backend = self::getBackend($deviceFile)))
			return NULL;
		return $backend->getImpl($deviceFile);
	}
}

/**
 * Factory to create objects of storage device classes.
 * @ingroup api
 */
class OMVStorageDeviceFactory extends OMVObject {
	/**
	 * Get the object of the class implementing the given storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return The object of the class implementing the given storage device,
	 *   otherwise NULL.
	 */
	public static function get($deviceFile) {
		return OMVStorageDevices::getImpl($deviceFile);
	}
}

/**
 * This class provides a simple interface to handle a LVM physical volume.
 * @ingroup api
 */
class OMVLvmPhysicalVolumes extends OMVObject {
	/**
	 * Get a list of physical volumes.
	 * @deprecated
	 * @return A list of physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate LVM physical volumes.
	 * @return A list of LVM physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; pvdisplay --noheadings -C -o pv_name ".
		  "2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		$list = array();
		// Parse command output:
		//   /dev/sdc
		//   /dev/sdd
		//   unknown device
		foreach($output as $outputk => $outputv) {
			$deviceFile = trim($outputv);
			if(!is_devicefile($deviceFile))
				continue;
			$list[] = $deviceFile;
		}
		return $list;
	}
}

/**
 * Helper class to get the LVM logical volumes.
 * @ingroup api
 */
class OMVLvmLogicalVolumes extends OMVObject {
	/**
	 * Get a list of logical volumes.
	 * @deprecated
	 * @return A list of logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate LVM logical volumes.
	 * @return A list of LVM logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; lvdisplay --noheadings --separator '|' ".
		  "-C -o vg_name,lv_name 2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		$list = array();
		foreach($output as $outputk => $outputv) {
			$outputv = explode("|", trim($outputv));
			// Replace '-' with '--' in volume group name.
			$list[] = sprintf("/dev/mapper/%s-%s",
			  str_replace("-", "--", $outputv[0]),
			  str_replace("-", "--", $outputv[1]));
			// Alternative device path: /dev/<vg_name>/<lv_name>
			//$list[] = sprintf("/dev/%s/%s", $outputv[0], $outputv[1]);
		}
		return $list;
	}
}
