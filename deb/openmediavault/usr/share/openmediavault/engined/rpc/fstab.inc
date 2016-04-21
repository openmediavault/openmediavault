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
class OMVRpcServiceFsTab extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "FsTab";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumerateEntries");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("getByFsName");
		$this->registerMethod("getByDir");
	}

	/**
	 * Enumerate all fstab mount point entries.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a mount point
	 *   entry.
	 */
	public function enumerateEntries($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.system.filesystem.mountpoint");
	}

	/**
	 * Get a mount point configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.system.filesystem.mountpoint",
		  $params['uuid']);
	}

	/**
	 * Set a mount point configuration object. The given mount point in the
	 * filesystem will be created.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em fsname The block special device or filesystem UUID to be mounted.
	 *   \em dir The mount point for the filesystem.
	 *   \em type The type of the filesystem.
	 *   \em opts The mount options associated with the filesystem.
	 *   \em freq This option is used for these filesystems by the dump(8)
	 *   command to determine which filesystems need to be dumped.
	 *   \em passno This field is used by the fsck(8) program to determine
	 *   the order in which filesystem checks are done at reboot time.
	 *   \em hidden Set to 0 to write the mount point entry to /etc/fstab.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.fstab.set");
		// Get the filesystem backend.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$fsbMngr->assertBackendExistsByType($params['type']);
		$fsb = $fsbMngr->getBackendByType($params['type']);
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject(
		  "conf.system.filesystem.mountpoint");
		$object->setAssoc($params);
		$object->set("hidden", !$fsb->hasFstabSupport());
		// Set the configuration object.
		$isNew = $object->isNew();
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify($isNew ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.filesystem.mountpoint",
		  $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a mount point configuration object. The mount point in the
	 * filesystem will be deleted.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function delete($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		return $this->deleteConfigObjectByUuid(
		  "conf.system.filesystem.mountpoint", $params['uuid'],
		  "org.openmediavault.conf.system.filesystem.mountpoint");
	}

	/**
	 * Get a mount point configuration object by the given file system name,
	 * which may be a UUID or block special device.
	 * @param params An array containing the following fields:
	 *   \em id The file system identifier, e.g. the UUID or block special
	 *   device file like <ul>
	 *   \li /dev/sda1
	 *   \li 02532317-cc35-421e-b750-c6a484fb109a
	 *   \li /dev/disk/by-uuid/02532317-cc35-421e-b750-c6a484fb109a
	 *   \li 4B04EA317E4AA567
	 *   \li /dev/disk/by-uuid/4B04EA317E4AA567
	 *   </ul>
	 * @param context The context of the caller.
	 * @return The mount point configuration object if exists, otherwise FALSE.
	 */
	public function getByFsName($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.fstab.getbyfsname");
		// Get the list of mount point configuration objects.
		$objects = $this->callMethod("enumerateEntries", NULL, $context);
		// Add function parameter to search argument list.
		$args = array();
		$args[] = $params['id'];
		// If the function parameter is a device file, then try to get the
		// file system UUID and add it to the search argument list. This is
		// done to increase the probability to find the fstab config object.
		if (is_devicefile($params['id'])) {
			$fs = \OMV\System\Filesystem\Filesystem::getImpl($params['id']);
			if (!is_null($fs) && $fs->exists() && $fs->hasUuid())
				$args[] = $fs->getUuid();
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
					// Check the name of the device special file, e.g.
					// - /dev/sdd1
					// - /dev/disk/by-uuid/4B04EA317E4AA567
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
			if (FALSE !== $result)
				break;
		}
		return $result;
	}

	/**
	 * Get a mount point configuration object by the given mount point.
	 * @param params An array containing the following fields:
	 *   \em dir The mount point.
	 * @param context The context of the caller.
	 * @return The mount point configuration object if exists, otherwise FALSE.
	 */
	public function getByDir($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.fstab.getbydir");
		// Get the list of mount point configuration objects.
		$objects = $this->callMethod("enumerateEntries", NULL, $context);
		// Try to find the configuration object matching the given search
		// arguments.
		$result = FALSE;
		foreach ($objects as $objectk => $objectv) {
			if ($params['dir'] === $objectv['dir']) {
				$result = $objectv;
				break;
			}
		}
		return $result;
	}
}
