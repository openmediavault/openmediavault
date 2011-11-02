<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/notify.inc");

/**
 * @class OMVModuleAbstract
 * The core module class.
 */
abstract class OMVModuleAbstract extends OMVObject {
	/**
	 * Get the name of the module.
	 * @return The name of the module.
	 */
	abstract public static function getName();
}

/**
 * @interface OMVIModuleStatus
 * Interface to get the module status, e.g. is the service enabled
 * and running.
 */
interface OMVIModuleStatus {
	/**
	 * Get the module status.
	 * @return An array containing the field following fields:
	 * name (string) The name of the module
	 * title (string) A title displayed in the WebGUI
	 * enabled (boolean) Is the implemented service enabled?
	 * running (boolean) Is the implemented service running?
	 */
	function getStatus();
}

/**
 * @class OMVModuleMgr
 */
class OMVModuleMgr extends OMVObject {
	private $map = array();

	/**
	 * Returns a module manager singleton.
	 * @return The module manager object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance)) {
			$instance = new OMVModuleMgr();
		}
		return $instance;
	}

	/**
	 * @param module Register a module.
	 * @return Returns TRUE on success or FALSE on failure.
	 */
	public function registerModule(OMVModuleAbstract $module) {
		if (!isset($module))
			return FALSE;
		$name = $module->getName();
		// Check if module already exists
		if (FALSE !== $this->getModule($name)) {
//			$this->debug("Module '{$name}' already registered");
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Module already registered (name=%s, class=%s)",
			  $name, get_class($module)));
		}
		$this->map[$name] = $module;
		return TRUE;
	}

	/**
	 * Get a module.
	 * @param name The name of the module to get.
	 * @return The module object or FALSE on failure.
	 */
	public function getModule($name) {
		if (!array_key_exists($name, $this->map))
			return FALSE;
		return $this->map[$name];
	}

	/**
	 * Get all registered modules.
	 * @return An array containing all registered modules.
	 */
	public function getModules() {
		return $this->map;
	}
}

// Include all module classes
$moduleDir = sprintf("%s/module", $GLOBALS['OMV_DOCUMENTROOT_DIR']);
foreach (new DirectoryIterator($moduleDir) as $item) {
	if ($item->isDot())
		continue;
	if ($item->isFile()) {
		$pathInfo = pathinfo($item->getFilename());
		if (strtolower($pathInfo['extension']) === "inc") {
			require_once("{$moduleDir}/{$item->getFilename()}");
		}
	}
}

// Bind listeners
$moduleMgr = &OMVModuleMgr::getInstance();
$modules = $moduleMgr->getModules();
$dispatcher = &OMVNotifyDispatcher::getInstance();
foreach ($modules as $module) {
	$dispatcher->addListener($module);
}
?>
