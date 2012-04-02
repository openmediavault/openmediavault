<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/functions.inc");

class FsTabRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"delete" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"getByFsName" => array(
				'{"type":"string"}'
			)
		);
	}

	/**
	 * Enumerate all fstab mount point entries.
	 * @return An array of objects. Each object represents a mount point
	 * entry.
	 */
	public function enumerateEntries() {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//system/fstab/mntent";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $objects;
	}

	/**
	 * Delete a mount point configuration object.
	 */
	public function delete($uuid) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Delete the configuration object
		$xpath = "//system/fstab/mntent[uuid='{$uuid}']";
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.fstab.mntent", $object);
	}

	/**
	 * Get a mount point configuration object by the given file system name,
	 * which may be a UUID or device file.
	 * @param id The name of the file system, e.g. UUID or device file.
	 * @return The mount point configuration object if exists, otherwise FALSE.
	 */
	public function getByFsName($id) {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the list of mount point configuration objects.
		$xpath = "//system/fstab/mntent";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add function parameter to search argument list.
		$args = array();
		$args[] = $id;
		// If the function parameter is a device file, then try to get the
		// file system UUID and add it to the search argument list. This is
		// done to increase the probability to find the fstab config object.
		if (is_devicefile($id)) {
			$fs = new OMVFilesystem($id);
			if ($fs->exists()) {
				$args[] = $fs->getUuid();
			}
		}
		// Try to find the configuration object matching the given search
		// arguments. Note, the field 'fsname' may contain a device file
		// or a file system UUID.
		$result = FALSE;
		foreach ($objects as $objectk => $objectv) {
			foreach ($args as $argk => $argv) {
				// Device files must be handled seperate because the device
				// may contain partitions.
				if (is_devicefile($argv)) {
					if (FALSE !== strpos($objectv['fsname'], $argv)) {
						$result = $objectv;
						break;
					}
				} else {
					if ($argv === $objectv['fsname']) {
						$result = $objectv;
						break;
					}
				}
			}
			if (TRUE === $result)
				break;
		}
		return $result;
	}
}
?>
