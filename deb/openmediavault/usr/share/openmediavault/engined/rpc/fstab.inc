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
namespace Engined\Rpc;

class FsTab extends \OMV\Rpc\ServiceAbstract {
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
	 *     command to determine which filesystems need to be dumped.
	 *   \em passno This field is used by the fsck(8) program to determine
	 *     the order in which filesystem checks are done at reboot time.
	 *   \em hidden Set to 0 to write the mount point entry to /etc/fstab.
	 *     Note, this flag is automatically pre-filled with the value
	 *     returned by the filesystem backend (which is specified via the
	 *     `type` parameter) when a new configuration object is created.
	 *   \em usagewarnthreshold The threshold in percent when to warn the
	 *      device is running out of disk space.
	 *   \em comment Any text string. This field is optional.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			// Check uniqueness:
			// - A file system can only be mounted once
			// - The mount point must be unique
			$db->assertIsUnique($object, "fsname");
			$db->assertIsUnique($object, "dir");
			// Ask the filesystem backend whether a '/etc/fstab' entry
			// is supported. If not, then mark this configuration
			// object as 'hidden'.
			$object->set("hidden", !$fsb->hasFstabSupport());
		}
		$db->set($object);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.filesystem.mountpoint",
			$params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get a mount point configuration object by the given file system name,
	 * which may be a UUID or block special device.
	 * @param params An array containing the following fields:
	 *   \em fsname The file system identifier, e.g. the UUID or block special
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.fstab.getbyfsname");
		// Get the values we are searching for.
		$fsNames = [];
		$fsNames[] = $params['fsname'];
		// If the file system exists, then get all available device
		// files and append them to the search filter to increase the
		// probability to find the mount point configuration object.
		$fs = \OMV\System\Filesystem\Filesystem::getImpl($params['fsname']);
		if (!is_null($fs) && $fs->exists()) {
			$fsNames[] = $fs->getCanonicalDeviceFile();
			$fsNames = array_merge($fsNames, $fs->getDeviceFileSymlinks());
			// To keep backward compatibility we need to search for
			// the file system UUID, too.
			if (TRUE === $fs->hasUuid()) {
				$fsNames[] = $fs->getUuid();
			}
			// Add the predictable device file, even if this might be
			// a duplication. Keep in mind that external file system
			// backends (plugins) are out of our control and might behave
			// unexpected here.
			$fsNames[] = $fs->getPredictableDeviceFile();
		}
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.filesystem.mountpoint", [
			"operator" => "stringEnum",
			"arg0" => "fsname",
			// Remove duplicates and re-index the array.
			"arg1" => array_values(array_unique($fsNames))
		]);
		if (empty($objects)) {
			return FALSE;
		}
		return $objects[0]->getAssoc();
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.fstab.getbydir");
		// Try to find the configuration object matching the given search
		// arguments.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.filesystem.mountpoint", [
			"operator" => "stringEquals",
			"arg0" => "dir",
			"arg1" => $params['dir']
		]);
		if (empty($objects)) {
			return FALSE;
		}
		return $objects[0]->getAssoc();
	}
}
