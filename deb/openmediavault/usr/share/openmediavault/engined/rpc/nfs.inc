<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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

class OMVRpcServiceNfs extends OMVRpcServiceAbstract {
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
	}

	/**
	 * Get the settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//services/nfs";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['enable'] = boolvalEx($object['enable']);
		$object['numproc'] = intval($object['numproc']);
		// Remove useless fields from object.
		unset($object['shares']);
		return $object;
	}

	/**
	 * Set the settings.
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
				  "numproc":{"type":"integer","minimum":1,"maximum":65535}
			  }
		  }');
		// Get the existing configuration object.
		$xpath = "//services/nfs";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the configuration object.
		$object = array_merge($object, array(
			  "enable" => array_boolval($params, 'enable'),
			  "numproc" => $params['numproc']
		  ));
		if(FALSE === $xmlConfig->replace("//services/nfs", $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.nfs", $object);
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
		// Get the configuration object.
		$xpath = "//services/nfs/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations.
		foreach($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object.
			$sfObject = $xmlConfig->get("//system/shares/sharedfolder[" .
			  "uuid='{$objectv['sharedfolderref']}']");
			if(!is_null($sfObject) && !empty($sfObject)) {
				$objectv['sharedfoldername'] = $sfObject['name'];
			}
		}
		// Filter the result.
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
		$xpath = sprintf("//services/nfs/shares/share[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set a share configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
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
				  "sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "mntentref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "client":{"type":"string"},
				  "options":{"type":"string"},
				  "extraoptions":{"type":"string"},
				  "comment":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"sharedfolderref" => $params['sharedfolderref'],
			"mntentref" => $params['mntentref'],
			"client" => $params['client'],
			"options" => $params['options'],
			"extraoptions" => $params['extraoptions'],
			"comment" => $params['comment']
		);
		// Set the configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Get the shared folder object.
			$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
			  $params['sharedfolderref']);
			$sfObject = $xmlConfig->get($xpath);
			if(is_null($sfObject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Get the mount point configuration object.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $sfObject['mntentref']);
			$meObject = $xmlConfig->get($xpath);
			if(is_null($meObject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Check if the required mount point already exists or if it is
			// necessary to create it.
			$dir = sprintf("%s/%s", $GLOBALS['OMV_NFSD_EXPORT_DIR'],
			  $sfObject['name']);
			// Try to get the mount point configuration object.
			$xpath = sprintf("//system/fstab/mntent[dir='%s']", $dir);
			$meObject2 = $xmlConfig->get($xpath);
			if(is_null($meObject2)) {
				// Prepare fstab mount point entry configuration object to
				// bind the shared folder directory into the NFS export
				// directory.
				$meObject2 = array(
					"uuid" => $GLOBALS['OMV_UUID_UNDEFINED'],
					"fsname" => sprintf("%s/%s", $meObject['dir'],
					  $sfObject['reldirpath']),
					"dir" => $dir,
					"type" => "none",
					"opts" => "bind",
					"freq" => 0,
					"passno" => 0
				);
				// Append associated fstab mount point entry configuration
				// object.
				$meObject2 = OMVRpc::exec("FsTab", "set", $meObject2,
				  $context);
			}
			// Modify the share configuration object.
			$object['mntentref'] = $meObject2['uuid'];
			// Append the share configuration object.
			$success = $xmlConfig->set("//services/nfs/shares",
			  array("share" => $object));
		} else {
			// Update the existing configuration object.
			$success = $xmlConfig->replace("//services/nfs/shares/share[" .
			  "uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.nfs.shares.share", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a share config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function deleteShare($params, $context) {
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
		$xpath = sprintf("//services/nfs/shares/share[uuid='%s']",
		  $params['uuid']);
		$object = $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.nfs.shares.share");
		// Check if the shared folder is still shared by another NFS share.
		// In this case do not delete or unmount the mount point configuration
		// object.
		$xpath = sprintf("//services/nfs/shares/share[mntentref='%s']",
		  $object['mntentref']);
		if(0 < $xmlConfig->count($xpath))
			return $object;
		// Delete the associated fstab mount point entry.
		OMVRpc::exec("FsTab", "delete", array("uuid" => $object['mntentref']),
		  $context);
		// Return the deleted configuration object.
		return $object;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceNfs());
?>
