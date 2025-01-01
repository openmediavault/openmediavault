<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
namespace Engined\Rpc;

require_once("openmediavault/functions.inc");

class FolderBrowser extends \OMV\Rpc\ServiceAbstract {
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
	 * @return array An array of directory names.
	 * @ŧhrow \OMV\Exception
	 */
	public function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.folderbrowser.get");
		// The field 'path' may not contain the characters '..'. This is
		// because of security reasons: the given canonicalized absolute
		// path MUST be below the given shared folder/mount point.
		if (1 == preg_match("/\.\./", $params['path'])) {
			throw new \OMV\Exception(
			  "The field 'path' contains forbidden two-dot symbols.");
		}
		switch ($params['type']) {
		case "sharedfolder":
			// Get the absolute shared folder path.
			$rootPath = \OMV\Rpc\Rpc::call("ShareMgmt", "getPath", [
				"uuid" => $params['uuid']
			], $context);
			break;
		case "mntent":
			// Get the mount point configuration object.
			$db = \OMV\Config\Database::getInstance();
			$object = $db->get("conf.system.filesystem.mountpoint",
				$params['uuid']);
			$rootPath = $object->get("dir");
			break;
		}
		// List the directory contents.
		$result = [];
		$path = build_path(DIRECTORY_SEPARATOR, $rootPath, $params['path']);
		if (file_exists($path)) {
			foreach (new \DirectoryIterator($path) as $item) {
				if ($item->isDot())
					continue;
				if (!$item->isDir())
					continue;
				$result[] = $item->getFilename();
			}
		}
		return $result;
	}
}
