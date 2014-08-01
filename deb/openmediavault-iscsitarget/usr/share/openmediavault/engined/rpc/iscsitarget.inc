<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceIscsiTarget extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "iSCSITarget";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getTargetList");
		$this->registerMethod("getTarget");
		$this->registerMethod("setTarget");
		$this->registerMethod("deleteTarget");
	}

	/**
	 * Get the configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//services/iscsitarget";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$object['authentication'] = !empty($object['authentication']['user']) ?
		  $object['authentication']['user'] : array();
		// Modify result object.
		$object['enable'] = boolvalEx($object['enable']);
		unset($object['targets']);
		return $object;
	}

	/**
	 * Set the configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
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
		  }');
		// Get existing configuration object.
		$xpath = "//services/iscsitarget";
		$oldObject = $xmlConfig->get($xpath);
		if(is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the configuration object.
		$object = array_merge($oldObject, array(
			  "enable" => array_boolval($params, 'enable'),
			  "extraoptions" => $params['extraoptions'],
			  "authentication" => !empty($params['authentication']) ?
				array("user" => $params['authentication']) : array()
		  ));
		// Update the authentication users uuid's (only necessary if the
		// user is new)
		if(!empty($object['authentication']['user'])) {
			foreach($object['authentication']['user'] as $userk => &$userv) {
				$userv['uuid'] = ($userv['uuid'] ==
				  $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $userv['uuid'];
			}
		}
		// Set the configuration object.
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.iscsitarget", $object, $oldObject);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get list of target configuration objects.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getTargetList($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = "//services/iscsitarget/targets/target";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations.
		$rdomainname = strtolower(implode(".", array_reverse(explode(".",
		  OMVNetwork::getHostname("--fqdn")))));
		foreach($objects as $objectk => &$objectv) {
			$objectv['iqn'] = sprintf("iqn.%s.%s", $objectv['activation'],
			  $rdomainname);
			if(!empty($objectv['identifier'])) {
				$objectv['iqn'] .= sprintf(":%s", $objectv['identifier']);
			}
			// Modify result object.
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
			if(empty($object['luns']['lun']))
				$object['luns'] = array();
			else {
				$object['luns'] = $object['luns']['lun'];
				foreach($object['luns'] as $lunk => &$lunv) {
					$lunv['id'] = intval($lunv['id']);
				}
			}
			if(empty($object['authentication']['user']))
				$object['authentication'] = array();
			else
				$object['authentication'] = $object['authentication']['user'];
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a target configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getTarget($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//services/iscsitarget/targets/target[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result object.
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
		if(empty($object['luns']['lun']))
			$object['luns'] = array();
		else {
			$object['luns'] = $object['luns']['lun'];
			foreach($object['luns'] as $lunk => &$lunv) {
				$lunv['id'] = intval($lunv['id']);
			}
		}
		if(empty($object['authentication']['user']))
			$object['authentication'] = array();
		else
			$object['authentication'] = $object['authentication']['user'];
		return $object;
	}

	/**
	 * Set a target configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setTarget($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
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
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"activation" => $params['activation'],
			"identifier" => $params['identifier'],
			"alias" => $params['alias'],
			"headerdigest" => $params['headerdigest'],
			"datadigest" => $params['datadigest'],
			"maxconnections" => $params['maxconnections'],
			"maxsessions" => $params['maxsessions'],
			"initialr2t" => $params['initialr2t'],
			"immediatedata" => $params['immediatedata'],
			"maxrecvdatasegmentlength" => $params['maxrecvdatasegmentlength'],
			"maxxmitdatasegmentlength" => $params['maxxmitdatasegmentlength'],
			"maxburstlength" => $params['maxburstlength'],
			"firstburstlength" => $params['firstburstlength'],
			"maxoutstandingr2t" => $params['maxoutstandingr2t'],
			"datapduinorder" => $params['datapduinorder'],
			"datasequenceinorder" => $params['datasequenceinorder'],
			"errorrecoverylevel" => $params['errorrecoverylevel'],
			"nopinterval" => $params['nopinterval'],
			"noptimeout" => $params['noptimeout'],
			"wthreads" => $params['wthreads'],
			"queuedcommands" => $params['queuedcommands'],
			"comment" => $params['comment'],
			"extraoptions" => $params['extraoptions'],
			"luns" => !empty($params['luns']) ?
			  array("lun" => $params['luns']) : array(),
			"authentication" => !empty($params['authentication']) ?
			  array("user" => $params['authentication']) : array()
		);
		// Update the LUNs uuid's (only necessary if the LUN is new).
		if(!empty($object['luns']['lun'])) {
			foreach($object['luns']['lun'] as $lunk => &$lunv) {
				$lunv['uuid'] = ($lunv['uuid'] ==
				  $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $lunv['uuid'];
			}
		}
		// Update the authentication users uuid's (only necessary if the
		// user is new).
		if(!empty($object['authentication']['user'])) {
			foreach($object['authentication']['user'] as $userk => &$userv) {
				$userv['uuid'] = ($userv['uuid'] ==
				  $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $userv['uuid'];
			}
		}
		// Set configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$object['activation'] = date("Y-m");
			// Append object to configuration.
			$success = $xmlConfig->set("//services/iscsitarget/targets",
			  array("target" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace(
			  "//services/iscsitarget/targets/target[uuid='{$params['uuid']}']",
			  $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.iscsitarget.targets.target", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a target configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteTarget($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Delete the configuration object.
		$xpath = sprintf("//services/iscsitarget/targets/target[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.iscsitarget.targets.target");
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceIscsiTarget());
?>
