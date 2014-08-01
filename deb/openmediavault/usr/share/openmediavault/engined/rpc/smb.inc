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
require_once("openmediavault/notify.inc");

class OMVRpcServiceSmb extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "SMB";
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
		$this->registerMethod("emptyRecycleBin");
		$this->registerMethod("getStats");
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
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['enable'] = boolvalEx($object['enable']);
		$object['loglevel'] = intval($object['loglevel']);
		$object['usesendfile'] = boolvalEx($object['usesendfile']);
		$object['aio'] = boolvalEx($object['aio']);
		$object['nullpasswords'] = boolvalEx($object['nullpasswords']);
		$object['localmaster'] = boolvalEx($object['localmaster']);
		$object['timeserver'] = boolvalEx($object['timeserver']);
		$object['winssupport'] = boolvalEx($object['winssupport']);
		$object['homesenable'] = boolvalEx($object['homesenable']);
		$object['homesbrowseable'] = boolvalEx($object['homesbrowseable']);
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
				  "workgroup":{"type":"string"},
				  "serverstring":{"type":"string"},
				  "loglevel":{"type":"integer","enum":[0,1,2,3,10]},
				  "usesendfile":{"type":"boolean"},
				  "aio":{"type":"boolean"},
				  "nullpasswords":{"type":"boolean"},
				  "localmaster":{"type":"boolean"},
				  "timeserver":{"type":"boolean"},
				  "winssupport":{"type":"boolean"},
				  "winsserver":{"type":"string"},
				  "homesenable":{"type":"boolean"},
				  "homesbrowseable":{"type":"boolean"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Get the existing configuration object.
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the configuration object.
		$object = array_merge($object, array(
			  "enable" => array_boolval($params, 'enable'),
			  "workgroup" => $params['workgroup'],
			  "serverstring" => $params['serverstring'],
			  "loglevel" => $params['loglevel'],
			  "usesendfile" => array_boolval($params, 'usesendfile'),
			  "aio" => array_boolval($params, 'aio'),
			  "nullpasswords" => array_boolval($params, 'nullpasswords'),
			  "localmaster" => array_boolval($params, 'localmaster'),
			  "timeserver" => array_boolval($params, 'timeserver'),
			  "winssupport" => array_boolval($params, 'winssupport'),
			  "winsserver" => $params['winsserver'],
			  "homesenable" => array_boolval($params, 'homesenable'),
			  "homesbrowseable" => array_boolval($params, 'homesbrowseable'),
			  "extraoptions" => $params['extraoptions']
		  ));
		if(FALSE === $xmlConfig->replace("//services/smb", $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smb", $object);
		// Return the configuration object.
		return $object;
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
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getShareList($params, $context) {
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
		// Get the configuration objects.
		$xpath = "//services/smb/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations.
		foreach($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Modify the result object.
			$objectv['enable'] = boolvalEx($objectv['enable']);
			$objectv['readonly'] = boolvalEx($objectv['readonly']);
			$objectv['browseable'] = boolvalEx($objectv['browseable']);
			$objectv['recyclebin'] = boolvalEx($objectv['recyclebin']);
			$objectv['recyclemaxsize'] = intval($objectv['recyclemaxsize']);
			$objectv['recyclemaxage'] = intval($objectv['recyclemaxage']);
			$objectv['hidedotfiles'] = boolvalEx($objectv['hidedotfiles']);
			$objectv['inheritacls'] = boolvalEx($objectv['inheritacls']);
			$objectv['inheritpermissions'] = boolvalEx(
			  $objectv['inheritpermissions']);
			$objectv['easupport'] = boolvalEx($objectv['easupport']);
			$objectv['storedosattributes'] = boolvalEx(
			  $objectv['storedosattributes']);
			$objectv['audit'] = boolvalEx($objectv['audit']);
			// Get the shared folder configuration object.
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[" .
			  "uuid='{$objectv['sharedfolderref']}']");
			if(!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
		}
		// Filter the result object list.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a share configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getShare($params, $context) {
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
		$xpath = sprintf("//services/smb/shares/share[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['enable'] = boolvalEx($object['enable']);
		$object['readonly'] = boolvalEx($object['readonly']);
		$object['browseable'] = boolvalEx($object['browseable']);
		$object['recyclebin'] = boolvalEx($object['recyclebin']);
		$object['recyclemaxsize'] = intval($object['recyclemaxsize']);
		$object['recyclemaxage'] = intval($object['recyclemaxage']);
		$object['hidedotfiles'] = boolvalEx($object['hidedotfiles']);
		$object['inheritacls'] = boolvalEx($object['inheritacls']);
		$object['inheritpermissions'] = boolvalEx($object['inheritpermissions']);
		$object['easupport'] = boolvalEx($object['easupport']);
		$object['storedosattributes'] = boolvalEx($object['storedosattributes']);
		$object['audit'] = boolvalEx($object['audit']);
		return $object;
	}

	/**
	 * Set a share configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setShare($params, $context) {
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
				  "enable":{"type":"boolean"},
				  "sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "name":{'.$GLOBALS['OMV_JSONSCHEMA_SHARENAME'].'},
				  "comment":{"type":"string"},
				  "guest":{"type":"string","enum":["no","allow","only"]},
				  "readonly":{"type":"boolean"},
				  "browseable":{"type":"boolean"},
				  "recyclebin":{"type":"boolean"},
				  "recyclemaxsize":{"type":"integer","minimum":0},
				  "recyclemaxage":{"type":"integer","minimum":0},
				  "hidedotfiles":{"type":"boolean"},
				  "inheritacls":{"type":"boolean"},
				  "inheritpermissions":{"type":"boolean"},
				  "easupport":{"type":"boolean"},
				  "storedosattributes":{"type":"boolean"},
				  "hostsallow":{"type":"string"},
				  "hostsdeny":{"type":"string"},
				  "audit":{"type":"boolean"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"enable" => array_boolval($params, 'enable'),
			"sharedfolderref" => $params['sharedfolderref'],
			"name" => $params['name'],
			"comment" => $params['comment'],
			"guest" => $params['guest'],
			"readonly" => array_boolval($params, 'readonly'),
			"browseable" => array_boolval($params, 'browseable'),
			"inheritacls" => array_boolval($params, 'inheritacls'),
			"inheritpermissions" => array_boolval($params,
			  'inheritpermissions'),
			"recyclebin" => array_boolval($params, 'recyclebin'),
			"recyclemaxsize" => $params['recyclemaxsize'],
			"recyclemaxage" => $params['recyclemaxage'],
			"hidedotfiles" => array_boolval($params, 'hidedotfiles'),
			"easupport" => array_boolval($params, 'easupport'),
			"storedosattributes" => array_boolval($params,
			  'storedosattributes'),
			"hostsallow" => $params['hostsallow'],
			"hostsdeny" => $params['hostsdeny'],
			"audit" => array_boolval($params, 'audit'),
			"extraoptions" => $params['extraoptions']
		);
		// Set the configuration object.
		$success = FALSE;
		if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness:
			// - Share name
			// - Shared folder
			$xpath = sprintf("//services/smb/shares/share[name='%s']",
			  $params['name']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  sprintf(gettext("A share with the name '%s' already exists"),
				  $params['name']));
			}
			$xpath = sprintf("//services/smb/shares/share[".
			  "sharedfolderref='%s']", $params['sharedfolderref']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  gettext("The shared folder is already used by another share."));
			}
			// Append object to the configuration.
			$success = $xmlConfig->set("//services/smb/shares",
			  array("share" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//services/smb/shares/share[" .
			  "uuid='{$params['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smb.shares.share", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a share configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteShare($params, $context) {
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
		$xpath = sprintf("//services/smb/shares/share[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.smb.shares.share");
	}

	/**
	 * Empty the recycle bin of the given shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_CONFIG_DIRTY
	 */
	public function emptyRecycleBin($params, $context) {
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
		// Check if the module is marked as dirty. This can be an indication
		// that the samba recycle bin cron script has not been created or
		// updated until now.
		if($this->isModuleDirty("samba"))
			throw new OMVException(OMVErrorMsg::E_CONFIG_DIRTY);
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Execute the rsync cron script.
			$cmd = "/bin/sh";
			$cmdArgs = array();
			$cmdArgs[] = sprintf("%s/samba-recycle-%s",
			  $GLOBALS['OMV_CRONSCRIPTS_DIR'], $params['uuid']);
			$cmdArgs[] = "-f";
			if(FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd." ".implode(" ", $cmdArgs), "");
			}
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Get statistics.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A string containing the requested statistics.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_EXEC_FAILED
	 */
	public function getStats($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Is service enabled?
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (!boolvalEx($object['enable'])) {
			$stats = gettext("Service disabled");
		} else {
			$cmd = "smbstatus -u 2>&1";
			if(0 !== $this->exec($cmd, $output)) {
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

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSmb());
?>
