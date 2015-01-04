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
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");

class OMVRpcServiceFolderBrowser extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "FolderBrowser";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
	}

	/**
	 * List the contents of the given directory. Note, only directories are
	 * listed.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em type The type of the configuration object, e.g. 'sharedfolder'
	 *   or 'mntent'.
	 *   \em path The relative directory path.
	 * @param context The context of the caller.
	 * @return An array of directory names.
	 * @throw E_RPC_SERVICE_INVALID_PARAM
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function get($params, $context) {
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
				  "type":{"type":"string","enum":["mntent","sharedfolder"]},
				  "path":{"type":"string"}
			  }
		  }');
		// The field 'path' may not contain the characters '..'. This is
		// because of security reasons: the given canonicalized absolute
		// path MUST be below the given shared folder/mount point.
		if (1 == preg_match("/\.\./", $params['path'])) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PARAM,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "path"));
		}
		switch ($params['type']) {
		case "sharedfolder":
			// Get the absolute shared folder path.
			$rootPath = OMVRpc::exec("ShareMgmt", "getPath", array(
			  "uuid" => $params['uuid']), $context);
			break;
		case "mntent":
			// Get the mount point configuration object.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $params['uuid']);
			$object = $xmlConfig->get($xpath);
			if (is_null($object)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			$rootPath = $object['dir'];
			break;
		}
		// List the directory contents.
		$result = array();
		$path = sprintf("%s/%s", $rootPath, rtrim($params['path'], "/"));
		foreach (new DirectoryIterator($path) as $item) {
			if ($item->isDot())
				continue;
			if (!$item->isDir())
				continue;
			$result[] = $item->getFilename();
		}
		return $result;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceFolderBrowser());
?>
