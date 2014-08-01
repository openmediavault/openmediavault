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
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceRsync extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Rsync";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("execute");
	}

	/**
	 * Get list of rsync job configuration objects.
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
	public function getList($params, $context) {
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
		$xpath = "//services/rsync/jobs/job";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations.
		foreach($objects as $objectk => &$objectv) {
			$objectv['srcname'] = gettext("n/a");
			$objectv['destname'] = gettext("n/a");
			switch($objectv['type']) {
			case "local":
				$sfobject = $xmlConfig->get("//system/shares/sharedfolder[".
				  "uuid='{$objectv['src']}']");
				if(!is_null($sfobject) && !empty($sfobject)) {
					$objectv['srcname'] = $sfobject['name'];
				}
				$sfobject = $xmlConfig->get("//system/shares/sharedfolder[".
				  "uuid='{$objectv['dest']}']");
				if(!is_null($sfobject) && !empty($sfobject)) {
					$objectv['destname'] = $sfobject['name'];
				}
				break;
			case "remote":
				switch($objectv['mode']) {
				case "push":
					$sfobject = $xmlConfig->get("//system/shares/" .
					  "sharedfolder[uuid='{$objectv['src']}']");
					if(!is_null($sfobject) && !empty($sfobject)) {
						$objectv['srcname'] = $sfobject['name'];
					}
					$objectv['destname'] = $objectv['dest'];
					break;
				case "pull":
					$objectv['srcname'] = $objectv['src'];
					$sfobject = $xmlConfig->get("//system/shares/" .
					  "sharedfolder[uuid='{$objectv['dest']}']");
					if(!is_null($sfobject) && !empty($sfobject)) {
						$objectv['destname'] = $sfobject['name'];
					}
					break;
				}
				break;
			}
			// Modify result data.
			$objectv['enable'] = boolvalEx($objectv['enable']);
			$objectv['sendemail'] = boolvalEx($objectv['sendemail']);
			$objectv['recursive'] = boolvalEx($objectv['recursive']);
			$objectv['times'] = boolvalEx($objectv['times']);
			$objectv['compress'] = boolvalEx($objectv['compress']);
			$objectv['archive'] = boolvalEx($objectv['archive']);
			$objectv['delete'] = boolvalEx($objectv['delete']);
			$objectv['quiet'] = boolvalEx($objectv['quiet']);
			$objectv['perms'] = boolvalEx($objectv['perms']);
			$objectv['acls'] = boolvalEx($objectv['acls']);
			$objectv['xattrs'] = boolvalEx($objectv['xattrs']);
			$objectv['everynminute'] = boolvalEx($objectv['everynminute']);
			$objectv['everynhour'] = boolvalEx($objectv['everynhour']);
			$objectv['everyndayofmonth'] = boolvalEx(
			  $objectv['everyndayofmonth']);
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a rsync job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function get($params, $context) {
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
		$xpath = sprintf("//services/rsync/jobs/job[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations.
		switch($object['type']) {
		case "local":
			$object['srcsharedfolderref'] = $object['src'];
			$object['destsharedfolderref'] = $object['dest'];
		case "remote":
			switch($object['mode']) {
			case "push":
				$object['srcsharedfolderref'] = $object['src'];
				$object['desturi'] = $object['dest'];
				break;
			case "pull":
				$object['srcuri'] = $object['src'];
				$object['destsharedfolderref'] = $object['dest'];
				break;
			}
			break;
		}
		// Modify result data.
		$object['enable'] = boolvalEx($object['enable']);
		$object['sendemail'] = boolvalEx($object['sendemail']);
		$object['recursive'] = boolvalEx($object['recursive']);
		$object['times'] = boolvalEx($object['times']);
		$object['compress'] = boolvalEx($object['compress']);
		$object['archive'] = boolvalEx($object['archive']);
		$object['delete'] = boolvalEx($object['delete']);
		$object['quiet'] = boolvalEx($object['quiet']);
		$object['perms'] = boolvalEx($object['perms']);
		$object['acls'] = boolvalEx($object['acls']);
		$object['xattrs'] = boolvalEx($object['xattrs']);
		$object['dryrun'] = boolvalEx($object['dryrun']);
		$object['partial'] = boolvalEx($object['partial']);
		$object['everynminute'] = boolvalEx($object['everynminute']);
		$object['everynhour'] = boolvalEx($object['everynhour']);
		$object['everyndayofmonth'] = boolvalEx($object['everyndayofmonth']);
		return $object;
	}

	/**
	 * Set a rsync job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function set($params, $context) {
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
				  "sendemail":{"type":"boolean"},
				  "comment":{"type":"string"},
				  "type":{"type":"string","enum":["local","remote"]},
				  "srcsharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].
					',"optional":true},
				  "srcuri":{"type":"string","optional":true},
				  "destsharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].
					',"optional":true},
				  "desturi":{"type":"string","optional":true},
				  "minute":{"type":"string","format":"regex",'.
					'"pattern":"\/^[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|'.
					'5[0-9]|[*]$\/"},
				  "everynminute":{"type":"boolean"},
				  "hour":{"type":"string","format":"regex",'.
					'"pattern":"\/^[0-9]|1[0-9]|2[0-3]|[*]$\/"},
				  "everynhour":{"type":"boolean"},
				  "month":{"type":"string","format":"regex",'.
					'"pattern":"\/^[1-9]|1[0-2]|[*]$\/"},
				  "dayofmonth":{"type":"string","format":"regex",'.
					'"pattern":"\/^[1-9]|1[0-9]|2[0-9]|3[0-1]|[*]$\/"},
				  "everyndayofmonth":{"type":"boolean"},
				  "dayofweek":{"type":"string","format":"regex",'.
					'"pattern":"\/^[1-7]|[*]$\/"},
				  "recursive":{"type":"boolean"},
				  "times":{"type":"boolean"},
				  "compress":{"type":"boolean"},
				  "archive":{"type":"boolean"},
				  "delete":{"type":"boolean"},
				  "quiet":{"type":"boolean"},
				  "perms":{"type":"boolean"},
				  "acls":{"type":"boolean"},
				  "xattrs":{"type":"boolean"},
				  "dryrun":{"type":"boolean"},
				  "partial":{"type":"boolean"},
				  "extraoptions":{"type":"string"},
				  "mode":{"type":"string","enum":["push","pull"],'.
					'"optional":true},
				  "password":{"type":"string","optional":true}
			  }
		  }');
		// Prepare the configuration data.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"enable" => array_boolval($params, 'enable'),
			"sendemail" => array_boolval($params, 'sendemail'),
			"comment" => $params['comment'],
			"type" => $params['type'],
			"src" => ($params['type'] === "local") ?
			  $params['srcsharedfolderref'] : (($params['mode'] === "push") ?
				$params['srcsharedfolderref'] : $params['srcuri']),
			"dest" => ($params['type'] === "local") ?
			  $params['destsharedfolderref'] : (($params['mode'] === "push") ?
				$params['desturi'] : $params['destsharedfolderref']),
			"minute" => $params['minute'],
			"everynminute" => array_boolval($params, 'everynminute'),
			"hour" => $params['hour'],
			"everynhour" => array_boolval($params, 'everynhour'),
			"dayofmonth" => $params['dayofmonth'],
			"everyndayofmonth" => array_boolval($params, 'everyndayofmonth'),
			"month" => $params['month'],
			"dayofweek" => $params['dayofweek'],
			"recursive" => array_boolval($params, 'recursive'),
			"times" => array_boolval($params, 'times'),
			"compress" => array_boolval($params, 'compress'),
			"archive" => array_boolval($params, 'archive'),
			"delete" => array_boolval($params, 'delete'),
			"quiet" => array_boolval($params, 'quiet'),
			"perms" => array_boolval($params, 'perms'),
			"acls" => array_boolval($params, 'acls'),
			"xattrs" => array_boolval($params, 'xattrs'),
			"dryrun" => array_boolval($params, 'dryrun'),
			"partial" => array_boolval($params, 'partial'),
			"extraoptions" => $params['extraoptions'],
			"mode" => isset($params['mode']) ? $params['mode'] : "",
			"password" => isset($params['password']) ? $params['password'] : ""
		);
		// Set the configuration data.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//services/rsync/jobs",
			  array("job" => $object));
		} else {
			// Update the existing configuration object.
			$success = $xmlConfig->replace("//services/rsync/jobs/job[" .
			  "uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(
		  ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.rsync.job", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a rsync job configuration object.
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
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Delete the configuration object.
		$xpath = sprintf("//services/rsync/jobs/job[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.rsync.job");
	}

	/**
	 * Execute a rsync job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the cron job to execute.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_CONFIG_DIRTY
	 * @throw E_EXEC_FAILED
	 */
	public function execute($params, $context) {
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
		// Check if the module is marked as dirty. This is an indication
		// that the rsync cron script has not been created or updated
		// until now.
		if($this->isModuleDirty("rsync"))
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
			$cmdArgs[] = sprintf("%s/rsync-%s",
			  $GLOBALS['OMV_CRONSCRIPTS_DIR'], $params['uuid']);
			if(FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd." ".implode(" ", $cmdArgs), "");
			}
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceRsync());
?>
