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
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");

class DirBrowserRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"get" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"type":{"type":"string","enum":["mntent","sharedfolder"]},
					"dir":{"type":"string"}
				}
			}')
		);
	}

	/**
	 * List the contents of the given directory. Note, only directories are
	 * listed.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em type The type of the configuration object, e.g. 'sharedfolder'
	 *   or 'mntent'.
	 *   \em dir The relative directory path.
	 * @return The directory contents.
	 */
	public function get($data) {
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
		// The field 'dir' may not contain the characters '..'. This is
		// because of security reasons: the given canonicalized absolute
		// path MUST be below the given shared folder/mount point.
		if (1 == preg_match("/\.\./", $data['dir'])) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PARAM,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "dir"));
		}
		switch ($data['type']) {
		case "sharedfolder":
			// Get the absolute shared folder path.
			$path = OMVRpc::exec("ShareMgmt", "getPath", array(
			  "uuid" => $data['uuid']));
			break;
		case "mntent":
			// Get the mount point configuration object.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']", $data['uuid']);
			$object = $xmlConfig->get($xpath);
			if (is_null($object)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			$path = $object['dir'];
			break;
		}
		// List the directory contents.
		$cmd = sprintf("sudo find %s -maxdepth 1 -type d -printf '%%f\n' 2>&1",
		  escapeshellarg(sprintf("%s/%s", $path, rtrim($data['dir'], "/"))));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		array_shift($output);
		return $output;
	}
}
?>
