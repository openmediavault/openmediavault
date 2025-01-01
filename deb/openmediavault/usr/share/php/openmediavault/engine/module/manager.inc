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
namespace OMV\Engine\Module;

require_once("openmediavault/functions.inc");

/**
 * The module manager.
 * @ingroup api
 */
class Manager {
	private $map = [];

	public function __construct() {
		// Get a list of all classes that are implementing a RPC service.
		$classes = get_declared_subclasses("\OMV\Engine\Module\ModuleAbstract");
		foreach ($classes as $classk => $classv) {
			$this->registerModule(new $classv());
		}
	}

	/**
	 * Returns a module manager singleton.
	 * @return object|null The module manager object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new Manager();
		return $instance;
	}

	/**
	 * Register a module.
	 * @param module The module class instance to be registered.
	 * @return bool Returns TRUE on success or FALSE on failure.
	 */
	private function registerModule(ModuleAbstract $module) {
		if (!isset($module))
			return FALSE;
		$name = $module->getName();
		// Check if module already exists.
		if (!is_null($existingModule = $this->getModule($name))) {
			throw new \OMV\Exception(
				"Failed to register module '%s' via '%s'. ".
				"It is already registered by '%s'.",
				$name, get_class($module), get_class($existingModule));
		}
		$this->map[mb_strtolower($name)] = $module;
		// Use optional alias?
		$alias = $module->getAlias();
		if (is_string($alias) && !empty($alias)) {
			// Check if module already exists.
			if (!is_null($existingModule = $this->getModule($alias))) {
				throw new \OMV\Exception(
					"Failed to register alias '%s' via '%s'. ".
					"It is already registered by '%s'.",
					$alias, get_class($module), get_class($existingModule));
			}
			$this->map[mb_strtolower($alias)] = $module;
		}
		ksort($this->map);
		return TRUE;
	}

	/**
	 * Get a module.
	 * @param name The name of the module to get.
	 * @return object|null The module instance or FALSE on failure.
	 */
	final public function getModule($name) {
		$name = mb_strtolower($name);
		if (!array_key_exists($name, $this->map))
			return NULL;
		return $this->map[$name];
	}

	/**
	 * Get all registered modules.
	 * @return array An array containing the instances of all
	 *   registered modules.
	 */
	final public function getModules() {
		return $this->map;
	}

	/**
	 * Helper function to get a list of dirty modules.
	 * @return array A sorted array containing the names of the modules
	 *   that are marked dirty.
	 */
	final public function getDirtyModules() {
		$jsonFile = new \OMV\Json\File(\OMV\Environment::get(
		  "OMV_ENGINED_DIRTY_MODULES_FILE"));
		if (!$jsonFile->exists())
			return [];
		$jsonFile->open("r", LOCK_SH);
		$modules = array_unique($jsonFile->read());
		$jsonFile->close();
		sort($modules, SORT_NATURAL);
		return $modules;
	}

	/**
	 * Helper function to get the names and descriptions of all dirty
	 * modules.
	 * @return array An associative array with name/desc pairs of the
	 *   dirty marked modules.
	 */
	final public function getDirtyModulesAssoc() {
		$names = $this->getDirtyModules();
		$descriptions = array_map(function($name) {
			$module = $this->getModule($name);
			return $module ? $module->getDescription() : "";
		}, $names);
		return array_combine($names, $descriptions);
	}

	/**
	 * Helper function to mark a module as dirty.
	 * @param name The name of the module.
	 * @return array The list of dirty modules.
	 */
	final public function setModuleDirty($name) {
		$jsonFile = new \OMV\Json\File(\OMV\Environment::get(
		  "OMV_ENGINED_DIRTY_MODULES_FILE"));
		$jsonFile->open("c+", LOCK_EX);
		// Read the dirty modules.
		// Note, if the file was newly created, then we must ensure that
		// the file contains valid JSON. A new created file is empty and
		// has a length of zero bytes.
		if ($jsonFile->isEmpty())
			$modules = [];
		else
			$modules = $jsonFile->read();
		// Append the specified dirty module.
		$modules[] = mb_strtolower($name);
		$modules = array_values(array_unique($modules));
		$jsonFile->write($modules);
		$jsonFile->close();
		return $modules;
	}

	/**
	 * Helper function to check whether a module is marked dirty.
	 * @param name The name of the module.
	 * @return bool TRUE if the module is marked dirty, otherwise FALSE.
	 */
	final public function isModuleDirty($name) {
		$modules = $this->getDirtyModules();
		return in_array(mb_strtolower($name), $modules);
	}

	/**
	 * Dump all registered modules.
	 */
	final public function dump() {
		print("Registered modules:\n");
		foreach ($this->map as $modulek => $modulev) {
			printf("  %s\n", $modulek);
		}
	}
}
