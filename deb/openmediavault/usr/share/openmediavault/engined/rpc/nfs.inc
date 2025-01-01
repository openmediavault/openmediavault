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

class Nfs extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "NFS";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getShareList");
		$this->registerMethod("getShare");
		$this->registerMethod("setShare");
		$this->registerMethod("deleteShare");
		$this->registerMethod("getStats");
	}

	/**
	 * Get the settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.nfs");
		$result = $object->getAssoc();
		$result["versions"] = explode_safe(",", $result["versions"]);
		return $result;
	}

	/**
	 * Set the settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.nfs.setsettings");
		$params["versions"] = implode(",", $params["versions"]);
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.nfs");
		$object->setAssoc($params);
		// Set the configuration object.
		$db->set($object);
		// Remove useless properties from the object.
		$object->remove("shares");
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get list of share configuration objects.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getShareList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.nfs.share");
		// Add additional share informations.
		$objectsAssoc = [];
		foreach ($objects as $objectk => &$objectv) {
			// Add the new property 'sharedfoldername'.
			$objectv->add("sharedfoldername", "string", gettext("n/a"));
			// Get the shared folder configuration object.
			$sfObject = $db->get("conf.system.sharedfolder",
			  $objectv->get("sharedfolderref"));
			// Update the 'sharedfoldername' property.
			$objectv->set("sharedfoldername", $sfObject->get("name"));
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a share configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.nfs.share", $params['uuid']);
	}

	/**
	 * Set a share configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.nfs.setshare");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.nfs.share");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			// Get the shared folder configuration object.
			$sfObject = $db->get("conf.system.sharedfolder",
			  $object->get("sharedfolderref"));
			// The shared folder 'reldirpath' must not contain any blanks
			// as this causes errors in SaltStack.
			// See https://github.com/saltstack/salt/issues/54508
			if (1 == preg_match("/ /", $sfObject->get("reldirpath"))) {
				throw new \InvalidArgumentException(sprintf(
					"The relative path of the shared folder '%s' contains blanks.",
					$sfObject->get("name")));
			}
			// Get the mount point configuration object.
			$meObject = $db->get("conf.system.filesystem.mountpoint",
			  $sfObject->get("mntentref"));
			// Check if the required mount point already exists or if it is
			// necessary to create it.
			$dir = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
			  "OMV_NFSD_EXPORT_DIR"), $sfObject->get("name"));
			// Try to get the mount point configuration object.
			$filter = [
				"operator" => "stringEquals",
				"arg0" => "dir",
				"arg1" => $dir
			];
			if (FALSE === $db->exists("conf.system.filesystem.mountpoint",
			  $filter)) {
				// Prepare mount point configuration object to bind the
				// shared folder directory into the NFS export directory.
				$meObject2 = new \OMV\Config\ConfigObject(
					"conf.system.filesystem.mountpoint");
				$meObject2->setNew();
				$meObject2->setAssoc([
					"fsname" => build_path(DIRECTORY_SEPARATOR,
						$meObject->get("dir"), $sfObject->get("reldirpath")),
					"dir" => $dir,
					"type" => "none",
					"opts" => implode(",", ["bind", \OMV\Environment::get(
						"OMV_FSTAB_MNTOPS_BIND")]),
					"freq" => 0,
					"passno" => 0
				]);
				// Put the mount point configuration object.
				$result = \OMV\Rpc\Rpc::call("FsTab", "set",
					$meObject2->getAssoc(), $context);
				// Update the configuration object with the values
				// returned by the RPC. Note, this is necessary to
				// get the correct object UUID.
				$meObject2->setAssoc($result);
			} else {
				// Load the existing mount point configuration object.
				$objects = $db->getByFilter(
					"conf.system.filesystem.mountpoint", $filter);
				$meObject2 = $objects[0];
			}
			// Modify the share configuration object.
			$object->set("mntentref", $meObject2->get("uuid"));
		}
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a share config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.nfs.share", $params['uuid']);
		$db->delete($object);
		// Check if the shared folder is still shared by another NFS share.
		// In this case do not delete or unmount the mount point configuration
		// object.
		$db = \OMV\Config\Database::getInstance();
		if (FALSE === $db->exists("conf.service.nfs.share", [
			"operator" => "stringEquals",
			"arg0" => "mntentref",
			"arg1" => $object->get('mntentref')
		])) {
			  // Delete the associated mount point.
			  \OMV\Rpc\Rpc::call("FsTab", "delete", [
				  "uuid" => $object->get('mntentref')
			  ], $context);
		  }
		// Return the deleted configuration object.
		return $object;
	}

	/**
	 * Get statistics.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A string containing the requested statistics.
	 */
	public function getStats($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		if ($this->isModuleDirty("nfs")) {
			throw new \OMV\Config\ConfigDirtyException();
		}
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.nfs");
		if (FALSE === $object->get("enable")) {
			$stats = gettext("Service disabled");
		} else {
			$cmdArgs = [];
			$cmdArgs[] = "-v";
			$cmd = new \OMV\System\Process("exportfs", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$stats = implode("\n", $output);
		}
		return $stats;
	}
}
