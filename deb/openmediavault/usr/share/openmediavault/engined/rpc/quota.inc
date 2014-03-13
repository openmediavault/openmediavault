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
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceQuota extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Quota";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
	}

    /**
	 * Get the quota for the given filesystem.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the filesystem.
	 * @param context The context of the caller.
	 * @return An array containing the requested quota configuration.
	 * @throw E_MISC_FAILURE
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
		// Get filesystem details.
		$fs = new OMVFilesystem($params['uuid']);
		if(!$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Filesystem '%s' not found"), $params['uuid']));
		}
		$devicefile = $fs->getDeviceFile();
		// Does any quota configuration exist for the given filesystem?
		$xpath = sprintf("//system/storage/filesystem/quota[fsuuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		// Prepare result. Iterate over all system users and groups and get
		// their quota configuration for the given filesystem.
		$result = array();
		// Get non-system users
		$users = OMVRpc::exec("UserMgmt", "enumerateUsers", NULL, $context);
		foreach($users as $userk => $userv) {
			// Set default values.
			$resObj = array(
				"type" => "user",
				"name" => $userv['name'],
				"bused" => 0,
				"bhardlimit" => 0,
				"bunit" => "MiB"
			);
			// Check if there is any configured quota for the given user.
			if(isset($object['usrquota']) && is_array($object['usrquota'])) {
				foreach($object['usrquota'] as $usrquotav) {
					if($userv['name'] !== $usrquotav['name'])
						continue;
					// Get user quotas and get the number of used disk space
					// if available. Note, 'blocks' are given in KiB.
					$sysUser = new OMVUser($userv['name']);
					if(FALSE !== ($quotas = $sysUser->getQuotas())) {
						if(array_key_exists($devicefile, $quotas)) {
							$resObj['bused'] = binary_format(
							  $quotas[$devicefile]['blocks'], array(
							  "fromPrefix" => "KiB"));
						}
					}
					// Convert the quota to a human readable unit. Note, the
					// values are stored a KiB in the configuration.
					$bhardlimit = binary_format($usrquotav['bhardlimit'],
					  array("fromPrefix" => "KiB", "maxPrefix" => "TiB",
					  "indexed" => TRUE));
					$resObj['bhardlimit'] = $bhardlimit['value'];
					$resObj['bunit'] = $bhardlimit['unit'];
				}
			}
			$result[] = $resObj;
		}
		// Get non-system groups
		$groups = OMVRpc::exec("UserMgmt", "enumerateGroups", NULL, $context);
		foreach($groups as $groupk => $groupv) {
			// Set default values.
			$resObj = array(
				"type" => "group",
				"name" => $groupv['name'],
				"bused" => 0,
				"bhardlimit" => 0,
				"bunit" => "MiB"
			);
			// Check if there is any configured quota for the given group.
			if(isset($object['grpquota']) && is_array($object['grpquota'])) {
				foreach($object['grpquota'] as $grpquotav) {
					if($groupv['name'] !== $grpquotav['name'])
						continue;
					// Get user quotas and get the number of used disk space
					// if available. Note, 'blocks' are given in KiB.
					$sysGroup = new OMVUserGroup($groupv['name']);
					if(FALSE !== ($quotas = $sysGroup->getQuotas())) {
						if(array_key_exists($devicefile, $quotas)) {
							$resObj['bused'] = binary_format(
							  $quotas[$devicefile]['blocks'], array(
							  "fromPrefix" => "KiB"));
						}
					}
					// Convert the quota to a human readable unit. Note, the
					// values are stored a KiB in the configuration.
					$bhardlimit = binary_format($grpquotav['bhardlimit'],
					  array("fromPrefix" => "KiB", "maxPrefix" => "TiB",
					  "indexed" => TRUE));
					$resObj['bhardlimit'] = $bhardlimit['value'];
					$resObj['bunit'] = $bhardlimit['unit'];
				}
			}
			$result[] = $resObj;
		}
		return $result;
	}

    /**
	 * Set the quota for the given filesystem.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the filesystem.
     *   \em quota. An array of quotas to set.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
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
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "quota":{
					  "type":"array",
					  "items":{
						  "type":"object",
						  "properties":{
							  "name":{"type":"string"},
							  "type":{"type":"string","enum":["user","group"]},
							  "bhardlimit":{"type":"number","minimum":0},
							  "bunit":{"type":"string","enum":["B","KiB",'.
								'"MiB","GiB","TiB","PiB","EiB"]}
						  }
					  }
				  }
			  }
		  }');
		// Create filesystem quota configuration object.
		$object = array();
		foreach($params['quota'] as $paramk => $paramv) {
			$quotaObj = array(
				"bsoftlimit" => 0,
				"bhardlimit" => binary_convert($paramv['bhardlimit'],
				  $paramv['bunit'], "KiB"),
				"isoftlimit" => 0,
				"ihardlimit" => 0
			);
			switch ($paramv['type']) {
			case "user":
				if(!isset($object['usrquota']))
					$object['usrquota'] = array();
				$quotaObj['name'] = $paramv['name'];
				$object['usrquota'][] = $quotaObj;
				break;
			case "group":
				if(!isset($object['grpquota']))
					$object['grpquota'] = array();
				$quotaObj['name'] = $paramv['name'];
				$object['grpquota'][] = $quotaObj;
				break;
			}
		}
		// Set configuration object.
		$xpath = sprintf("//system/storage/filesystem/quota[fsuuid='%s']",
		  $params['uuid']);
		if(FALSE === $xmlConfig->exists($xpath)) {
			// Create a new configuration object.
			$object['uuid'] = OMVUtil::uuid();
			$object['fsuuid'] = $params['uuid'];
			// Append object to configuration.
			$success = $xmlConfig->set("//system/storage/filesystem",
			  array("quota" => $object));
		} else {
			// Get existing configuration object.
			$oldObject = $xmlConfig->get($xpath);
			if(is_null($oldObject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			$object['uuid'] = $oldObject['uuid'];
			$object['fsuuid'] = $oldObject['fsuuid'];
			// Update existing configuration object.
			$success = $xmlConfig->replace($xpath, $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.filesystem.quota", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a filesystem quota configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	function delete($params, $context) {
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
		$xpath = sprintf("//system/storage/filesystem/quota[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.storage.filesystem.quota");
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceQuota());
?>
