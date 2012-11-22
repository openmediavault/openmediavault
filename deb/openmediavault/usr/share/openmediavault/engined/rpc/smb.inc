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

class SMBRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"workgroup":{"type":"string"},
					"serverstring":{"type":"string"},
					"loglevel":{"type":"integer","enum":[0,1,2,3,10]},
					"usesendfile":{"type":"boolean"},
					"nullpasswords":{"type":"boolean"},
					"localmaster":{"type":"boolean"},
					"timeserver":{"type":"boolean"},
					"winssupport":{"type":"boolean"},
					"winsserver":{"type":"string"},
					"homesenable":{"type":"boolean"},
					"homesbrowseable":{"type":"boolean"},
					"extraoptions":{"type":"string"},
					"dnssdenable":{"type":"boolean"},
					"dnssdname":{"type":"string"}
				}
			}'),
			"getShareList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"name":{'.$GLOBALS['OMV_JSONSCHEMA_SHARENAME'].'},
					"comment":{"type":"string"},
					"guestok":{"type":"boolean"},
					"readonly":{"type":"boolean"},
					"browseable":{"type":"boolean"},
					"recyclebin":{"type":"boolean"},
					"hidedotfiles":{"type":"boolean"},
					"inheritacls":{"type":"boolean"},
					"inheritpermissions":{"type":"boolean"},
					"easupport":{"type":"boolean"},
					"storedosattributes":{"type":"boolean"},
					"hostsallow":{"type":"string"},
					"hostsdeny":{"type":"string"},
					"extraoptions":{"type":"string"}
				}
			}'),
			"deleteShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}')
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
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['loglevel'] = intval($object['loglevel']);
		$object['usesendfile'] = boolval($object['usesendfile']);
		$object['nullpasswords'] = boolval($object['nullpasswords']);
		$object['localmaster'] = boolval($object['localmaster']);
		$object['timeserver'] = boolval($object['timeserver']);
		$object['winssupport'] = boolval($object['winssupport']);
		$object['homesenable'] = boolval($object['homesenable']);
		$object['homesbrowseable'] = boolval($object['homesbrowseable']);
		$object['dnssdenable'] = boolval($object['dnssd']['enable']);
		$object['dnssdname'] = $object['dnssd']['name'];
		// Remove useless data
		unset($object['dnssd']);
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
		$xpath = "//services/smb";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"workgroup" => $data['workgroup'],
			"serverstring" => $data['serverstring'],
			"loglevel" => $data['loglevel'],
			"usesendfile" => array_boolval($data, 'usesendfile'),
			"nullpasswords" => array_boolval($data, 'nullpasswords'),
			"localmaster" => array_boolval($data, 'localmaster'),
			"timeserver" => array_boolval($data, 'timeserver'),
			"winssupport" => array_boolval($data, 'winssupport'),
			"winsserver" => $data['winsserver'],
			"homesenable" => array_boolval($data, 'homesenable'),
			"homesbrowseable" => array_boolval($data, 'homesbrowseable'),
			"extraoptions" => $data['extraoptions'],
			"shares" => isset($oldObject['shares']) ?
			  $oldObject['shares'] : array(),
			"dnssd" => array(
				"enable" => array_boolval($data, 'dnssdenable'),
				"name" => $data['dnssdname']
			)
		);
		if (FALSE === $xmlConfig->replace("//services/smb", $object)) {
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
		  "org.openmediavault.services.smb", $object);
	}

	/**
	 * Get list of share configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getShareList($data) {
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
		$xpath = "//services/smb/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations
		foreach ($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Modify result data
			$objectv['guestok'] = boolval($objectv['guestok']);
			$objectv['readonly'] = boolval($objectv['readonly']);
			$objectv['browseable'] = boolval($objectv['browseable']);
			$objectv['recyclebin'] = boolval($objectv['recyclebin']);
			$objectv['hidedotfiles'] = boolval($objectv['hidedotfiles']);
			$objectv['inheritacls'] = boolval($objectv['inheritacls']);
			$objectv['inheritpermissions'] = boolval(
			  $objectv['inheritpermissions']);
			$objectv['easupport'] = boolval($objectv['easupport']);
			$objectv['storedosattributes'] = boolval(
			  $objectv['storedosattributes']);
			// Get the shared folder configuration object
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[" .
			  "uuid='{$objectv['sharedfolderref']}']");
			if (!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a share configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	public function getShare($data) {
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
		$xpath = sprintf("//services/smb/shares/share[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['guestok'] = boolval($object['guestok']);
		$object['readonly'] = boolval($object['readonly']);
		$object['browseable'] = boolval($object['browseable']);
		$object['recyclebin'] = boolval($object['recyclebin']);
		$object['hidedotfiles'] = boolval($object['hidedotfiles']);
		$object['inheritacls'] = boolval($object['inheritacls']);
		$object['inheritpermissions'] = boolval($object['inheritpermissions']);
		$object['easupport'] = boolval($object['easupport']);
		$object['storedosattributes'] = boolval($object['storedosattributes']);
		return $object;
	}

	/**
	 * Set a share configuration object.
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
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"sharedfolderref" => $data['sharedfolderref'],
			"name" => $data['name'],
			"comment" => $data['comment'],
			"guestok" => array_boolval($data, 'guestok'),
			"readonly" => array_boolval($data, 'readonly'),
			"browseable" => array_boolval($data, 'browseable'),
			"inheritacls" => array_boolval($data, 'inheritacls'),
			"inheritpermissions" => array_boolval($data, 'inheritpermissions'),
			"recyclebin" => array_boolval($data, 'recyclebin'),
			"hidedotfiles" => array_boolval($data, 'hidedotfiles'),
			"easupport" => array_boolval($data, 'easupport'),
			"storedosattributes" => array_boolval($data, 'storedosattributes'),
			"hostsallow" => $data['hostsallow'],
			"hostsdeny" => $data['hostsdeny'],
			"extraoptions" => $data['extraoptions']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness
			if (TRUE === $xmlConfig->exists("//services/smb/shares/share[" .
			  "name='{$data['name']}']")) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  gettext("A share with the given name already exists"));
			}
			// Append object to configuration
			$success = $xmlConfig->set("//services/smb/shares",
			  array("share" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//services/smb/shares/share[" .
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
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smb.shares.share", $object);
	}

	/**
	 * Delete a share configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function deleteShare($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Delete the configuration object.
		$xpath = sprintf("//services/smb/shares/share[uuid='%s']",
		  $data['uuid']);
		$this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.smb.shares.share");
	}

	/**
	 * Get statistics.
	 */
	public function getStats() {
		global $xmlConfig;
		// Is service enabled?
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (!array_boolval($object, 'enable')) {
			$stats = gettext("Service disabled");
		} else {
			$cmd = "sudo smbstatus -u 2>&1";
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			array_shift($output);
			array_shift($output);
			$stats = implode("\n", $output);
		}
		return $stats;
	}
}
?>
