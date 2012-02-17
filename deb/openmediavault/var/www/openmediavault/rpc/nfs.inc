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
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class NFSRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"}
				}
			}'),
			"getShareList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"getShare" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"mntentref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"client":{"type":"string"},
					"options":{"type":"string"},
					"extraoptions":{"type":"string"},
					"comment":{"type":"string"}
				}
			}'),
			"deleteShare" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			)
		);
	}

	public function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/nfs";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		unset($object['shares']);
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		return $object;
	}

	public function setSettings($data) {
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
		// Get existing configuration object
		$xpath = "//services/nfs";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"shares" => isset($oldObject['shares']) ?
				$oldObject['shares'] : array()
		);
		if (FALSE === $xmlConfig->replace("//services/nfs", $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.nfs", $object);
	}

	/**
	 * Get list of share config objects.
	 */
	public function getShareList($start, $count, $sortField, $sortDir) {
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
		// Get configuration object
		$xpath = "//services/nfs/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations
		foreach ($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[" .
			  "uuid='{$objectv['sharedfolderref']}']");
			if (!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get a share config object.
	 */
	public function getShare($uuid) {
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
		// Get configuration object
		$xpath = sprintf("//services/nfs/shares/share[uuid='%s']", $uuid);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set a share config object.
	 */
	public function setShare($data) {
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
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"sharedfolderref" => $data['sharedfolderref'],
			"client" => $data['client'],
			"options" => $data['options'],
			"extraoptions" => $data['extraoptions'],
			"comment" => $data['comment']
		);
		// Set configuration object.
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Get the shared folder object.
			$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
			  $data['sharedfolderref']);
			$sfobject = $xmlConfig->get($xpath);
			if (is_null($sfobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Get the mount point configuration object.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $sfobject['mntentref']);
			$meobject = $xmlConfig->get($xpath);
			if (is_null($meobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Check if the required mount point already exists or if it is
			// necessary to create it.
			$dir = sprintf("%s/%s", $GLOBALS['OMV_NFSD_EXPORT_DIR'],
			  $sfobject['name']);
			// Try to get the mount point configuration object.
			$xpath = sprintf("//system/fstab/mntent[dir='%s']", $dir);
			$meobject2 = $xmlConfig->get($xpath);
			if (is_null($meobject2)) {
				// Prepare fstab mount point entry configuration object to
				// bind the shared folder directory into the NFS export
				// directory.
				$meobject2 = array(
					"uuid" => OMVUtil::uuid(),
					"fsname" => sprintf("%s/%s", $meobject['dir'],
					  $sfobject['reldirpath']),
					"dir" => $dir,
					"type" => "none",
					"opts" => "bind",
					"freq" => 0,
					"passno" => 0
				);
				// Append associated fstab mount point entry configuration
				// object.
				if (FALSE === $xmlConfig->set("//system/fstab", array(
				  "mntent" => $meobject2))) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
				}
			}
			// Modify the share configuration object.
			$object['mntentref'] = $meobject2['uuid'];
			// Append the share configuration object.
			$success = $xmlConfig->set("//services/nfs/shares",
			  array("share" => $object));
		} else {
			// Update the existing configuration object.
			$success = $xmlConfig->replace("//services/nfs/shares/share[" .
			  "uuid='{$data['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$dispatcher->notify(OMV_NOTIFY_CREATE,
			  "org.openmediavault.system.fstab.mntent", $meobject2);
		}
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.nfs.shares.share", $object);
	}

	/**
	 * Delete a share config object.
	 */
	public function deleteShare($uuid) {
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
		// Get the object
		$xpath = sprintf("//services/nfs/shares/share[uuid='%s']", $uuid);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.services.nfs.shares.share", $object);
		// Delete the configuration object.
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save the configuration.
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes.
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.nfs.shares.share", $object);
		// Delete the associated fstab mount point entry.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		if (FALSE === ($meObject = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save the configuration.
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes.
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.fstab.mntent", $meObject);
	}
}
?>
