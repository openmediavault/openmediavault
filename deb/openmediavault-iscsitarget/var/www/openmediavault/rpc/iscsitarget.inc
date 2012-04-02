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
require_once("openmediavault/system.inc");
require_once("openmediavault/notify.inc");

class iSCSITargetRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"extraoptions":{"type":"string"},
					"authentication":{
						"type":"array",
						"items":{
							"type":"object",
							"properties":{
								"uuid":{'.
								  $GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
								"type":{"type":"string","enum":'.
								  '["incoming","outgoing"]},
								"username":{"type":"string"},
								"password":{"type":"string"}
							}
						}
					}
				}
			}'),
			"getTargetList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"getTarget" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setTarget" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"activation":{"type":"string","format":"regex","pattern":'.
					  '"#^([0-9]{4}-[0-9]{2})|$#"},
					"identifier":{"type":"string"},
					"alias":{"type":"string"},
					"headerdigest":{"type":"string","enum":["CRC32C","None"]},
					"datadigest":{"type":"string","enum":["CRC32C","None"]},
					"maxconnections":{"type":"integer","minimum":0},
					"maxsessions":{"type":"integer","minimum":0},
					"initialr2t":{"type":"boolean"},
					"immediatedata":{"type":"boolean"},
					"maxrecvdatasegmentlength":{"type":"integer","minimum":0},
					"maxxmitdatasegmentlength":{"type":"integer","minimum":0},
					"maxburstlength":{"type":"integer","minimum":0},
					"firstburstlength":{"type":"integer","minimum":0},
					"maxoutstandingr2t":{"type":"integer","minimum":0},
					"datapduinorder":{"type":"boolean"},
					"datasequenceinorder":{"type":"boolean"},
					"errorrecoverylevel":{"type":"integer","minimum":0},
					"nopinterval":{"type":"integer","minimum":0},
					"noptimeout":{"type":"integer","minimum":0},
					"wthreads":{"type":"integer","minimum":0},
					"queuedcommands":{"type":"integer","minimum":0},
					"comment":{"type":"string"},
					"extraoptions":{"type":"string"},
					"authentication":{
						"type":"array",
						"items":{
							"type":"object",
							"properties":{
								"uuid":{'.
								  $GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
								"type":{"type":"string","enum":'.
								  '["incoming","outgoing"]},
								"username":{"type":"string"},
								"password":{"type":"string"}
							}
						}
					},
					"luns":{
						"type":"array",
						"items":{
							"type":"object",
							"properties":{
								"uuid":{'.
								  $GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
								"id":{"type":"integer","minimum":0},
								"type":{"type":"string","enum":'.
								  '["fileio","blockio"]},
								"iomode":{"type":"string","enum":'.
								  '["wt","wb","ro"]},
								"devicefile":{'.
								  $GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
								"scsiid":{"type":"string"},
								"scsisn":{"type":"string"}
							}
						}
					}
				}
			}'),
			"deleteTarget" => array(
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
		$xpath = "//services/iscsitarget";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$object['authentication'] = !empty($object['authentication']['user']) ?
		  $object['authentication']['user'] : array();
		unset($object['targets']);
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
		$xpath = "//services/iscsitarget";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"extraoptions" => $data['extraoptions'],
			"targets" => isset($oldObject['targets']) ?
			  $oldObject['targets'] : array(),
			"authentication" => !empty($data['authentication']) ?
			  array("user" => $data['authentication']) : array()
		);
		// Update the authentication users uuid's (only necessary if the
		// user is new)
		if (!empty($object['authentication']['user'])) {
			foreach ($object['authentication']['user'] as
			  $objectk => &$objectv) {
				$objectv['uuid'] = ($objectv['uuid'] ==
				  $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $objectv['uuid'];
			}
		}
		if (FALSE === $xmlConfig->replace("//services/iscsitarget", $object)) {
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
		  "org.openmediavault.services.iscsitarget", $object);
	}

	/**
	 * Get list of target config objects.
	 */
	public function getTargetList($start, $count, $sortField, $sortDir) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/iscsitarget/targets/target";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations
		$rdomainname = strtolower(implode(".", array_reverse(explode(".",
		  OMVNetwork::getHostname("--fqdn")))));
		foreach ($objects as $objectk => &$objectv) {
			$objectv['iqn'] = sprintf("iqn.%s.%s", $objectv['activation'],
			  $rdomainname);
			if (!empty($objectv['identifier'])) {
				$objectv['iqn'] .= sprintf(":%s", $objectv['identifier']);
			}
			// Modify result data
			$objectv['maxconnections'] = intval($objectv['maxconnections']);
			$objectv['maxsessions'] = intval($objectv['maxsessions']);
			$objectv['maxrecvdatasegmentlength'] = intval(
			  $objectv['maxrecvdatasegmentlength']);
			$objectv['maxxmitdatasegmentlength'] = intval(
			  $objectv['maxxmitdatasegmentlength']);
			$objectv['maxburstlength'] = intval($objectv['maxburstlength']);
			$objectv['firstburstlength'] = intval(
			  $objectv['firstburstlength']);
			$objectv['maxoutstandingr2t'] = intval(
			  $objectv['maxoutstandingr2t']);
			$objectv['errorrecoverylevel'] = intval(
			  $objectv['errorrecoverylevel']);
			$objectv['nopinterval'] = intval($objectv['nopinterval']);
			$objectv['noptimeout'] = intval($objectv['noptimeout']);
			$objectv['wthreads'] = intval($objectv['wthreads']);
			$objectv['queuedcommands'] = intval($objectv['queuedcommands']);
			foreach ($objectv['luns']['lun'] as $lunk => &$lunv) {
				$lunv['id'] = intval($lunv['id']);
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get a target config object.
	 */
	public function getTarget($uuid) {
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
		$xpath = "//services/iscsitarget/targets/target[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['maxconnections'] = intval($object['maxconnections']);
		$object['maxsessions'] = intval($object['maxsessions']);
		$object['maxrecvdatasegmentlength'] = intval(
		  $object['maxrecvdatasegmentlength']);
		$object['maxxmitdatasegmentlength'] = intval(
		  $object['maxxmitdatasegmentlength']);
		$object['maxburstlength'] = intval($object['maxburstlength']);
		$object['firstburstlength'] = intval($object['firstburstlength']);
		$object['maxoutstandingr2t'] = intval($object['maxoutstandingr2t']);
		$object['errorrecoverylevel'] = intval($object['errorrecoverylevel']);
		$object['nopinterval'] = intval($object['nopinterval']);
		$object['noptimeout'] = intval($object['noptimeout']);
		$object['wthreads'] = intval($object['wthreads']);
		$object['queuedcommands'] = intval($object['queuedcommands']);
		foreach ($object['luns']['lun'] as $lunk => &$lunv) {
			$lunv['id'] = intval($lunv['id']);
		}
		return $object;
	}

	/**
	 * Set a target config object.
	 */
	public function setTarget($data) {
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
			"activation" => $data['activation'],
			"identifier" => $data['identifier'],
			"alias" => $data['alias'],
			"headerdigest" => $data['headerdigest'],
			"datadigest" => $data['datadigest'],
			"maxconnections" => $data['maxconnections'],
			"maxsessions" => $data['maxsessions'],
			"initialr2t" => $data['initialr2t'],
			"immediatedata" => $data['immediatedata'],
			"maxrecvdatasegmentlength" => $data['maxrecvdatasegmentlength'],
			"maxxmitdatasegmentlength" => $data['maxxmitdatasegmentlength'],
			"maxburstlength" => $data['maxburstlength'],
			"firstburstlength" => $data['firstburstlength'],
			"maxoutstandingr2t" => $data['maxoutstandingr2t'],
			"datapduinorder" => $data['datapduinorder'],
			"datasequenceinorder" => $data['datasequenceinorder'],
			"errorrecoverylevel" => $data['errorrecoverylevel'],
			"nopinterval" => $data['nopinterval'],
			"noptimeout" => $data['noptimeout'],
			"wthreads" => $data['wthreads'],
			"queuedcommands" => $data['queuedcommands'],
			"comment" => $data['comment'],
			"extraoptions" => $data['extraoptions'],
			"luns" => !empty($data['luns']) ?
			  array("lun" => $data['luns']) : array(),
			"authentication" => !empty($data['authentication']) ?
			  array("user" => $data['authentication']) : array()
		);
		// Update the LUNs uuid's (only necessary if the LUN is new)
		if (!empty($object['luns']['lun'])) {
			foreach ($object['luns']['lun'] as $objectk => &$objectv) {
				$objectv['uuid'] = ($objectv['uuid'] ==
				  $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $objectv['uuid'];
			}
		}
		// Update the authentication users uuid's (only necessary if the
		// user is new)
		if (!empty($object['authentication']['user'])) {
			foreach ($object['authentication']['user'] as $objectk =>
			  &$objectv) {
				$objectv['uuid'] = ($objectv['uuid'] ==
				  $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $objectv['uuid'];
			}
		}
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$object['activation'] = date("Y-m");
			// Append object to configuration
			$success = $xmlConfig->set("//services/iscsitarget/targets",
			  array("target" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace(
			  "//services/iscsitarget/targets/target[uuid='{$data['uuid']}']",
			  $object);
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
		  "org.openmediavault.services.iscsitarget.targets.target", $object);
	}

	/**
	 * Delete a target config object.
	 */
	public function deleteTarget($uuid) {
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
		// Delete configuration object
		$xpath = "//services/iscsitarget/targets/target[uuid='{$uuid}']";
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
		  "org.openmediavault.services.iscsitarget.targets.target", $object);
	}
}
?>
