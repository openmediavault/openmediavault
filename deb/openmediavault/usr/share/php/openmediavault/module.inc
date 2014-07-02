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
require_once("openmediavault/globals.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/notify.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/functions.inc");

/**
 * The core module class.
 * @ingroup api
 */
abstract class OMVModuleAbstract extends OMVObject {
	/**
	 * Get the name of the module.
	 * @return The name of the module.
	 */
	abstract public function getName();

	/**
	 * Helper function to mark this module as dirty.
	 * @return The list of dirty modules.
	 */
	final public function setDirty() {
		return $this->setDirtyByName($this->getName());
	}

	/**
	 * Helper function to mark the given module as dirty.
	 * @param name The name of the module.
	 * @return The list of dirty modules.
	 */
	final public function setDirtyByName($name) {
		$moduleMgr = &OMVModuleMgr::getInstance();
		return $moduleMgr->setModuleDirty($name);
	}

	/**
	 * Helper function to check whether the module is marked dirty.
	 * @return TRUE if the module is marked dirty, otherwise FALSE.
	 */
	final public function isDirty() {
		$moduleMgr = &OMVModuleMgr::getInstance();
		return $moduleMgr->isModuleDirty($this->getName());
	}

	/**
	 * Helper function to execute an external program.
	 * @param command The command that will be executed.
	 * @param output If the output argument is present, then the specified
	 *   array will be filled with every line of output from the command.
	 *   Trailing whitespace, such as \n, is not included in this array.
	 * @return The exit code of the command.
	 */
	final protected function exec($command, &$output = NULL) {
		OMVUtil::exec($command, $output, $result);
		return $result;
	}

	/**
	 * Helper function to get the administrator context.
	 * @return The administrator context object.
	 */
	final protected function getAdminContext() {
		return array(
			  "username" => "admin",
			  "role" => OMV_ROLE_ADMINISTRATOR
		  );
	}

	/**
	 * Add a task that can be performed at a later date.
	 * @param id The identifier of the task type.
	 * @param name The name of the class method to be executed.
	 * @param params The parameters passed to the method when it is executed.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 */
	final protected function addTask($id, $name, $params) {
		// Check if class method exists.
		if (!method_exists($this, $name)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("The method '%s' does not exist for module '%s'",
			  $name, $this->getName()));
		}
		// Add new task to the modules pending task list. Append
		// the new task if the file already exists, otherwise
		// create a new file.
		$filename = sprintf("/var/lib/openmediavault/%s_tasks.json",
		  $this->getName());
		$jsonFile = new OMVJsonFile($filename);
		$exists = $jsonFile->exists();
		$jsonFile->open("c+");
		if ($exists)
			$tasks = $jsonFile->read();
		else
			$tasks = array();
		// Add the new task to the list of pending tasks.
		$tasks[] = array(
			"id" => $id,
			"func" => $name,
			"params" => $params
		);
		// Write the pending task list.
		$jsonFile->write($tasks);
		$jsonFile->close();
	}

	/**
	 * Execute the tasks with the given identifier.
	 * @param id The identifier of the tasks to be executed.
	 * @return None.
	 */
	final protected function execTasks($id) {
		$filename = sprintf("/var/lib/openmediavault/%s_tasks.json",
		  $this->getName());
		$jsonFile = new OMVJsonFile($filename);
		if (!$jsonFile->exists())
			return;
		$jsonFile->open("c+");
		$tasks = $jsonFile->read();
		// Get list of tasks to be executed.
		$runTasks = array_filter_ex($tasks, "id", $id);
		foreach ($runTasks as $taskk => $taskv) {
			call_user_func_array(array($this, $taskv['func']), array(
			  $taskv['params']));
		}
		// Remove executed tasks from list. Note, the array must be
		// re-indexed.
		$tasks = array_values(array_diff_key($tasks, $runTasks));
		// Write back list of not executed task.
		$jsonFile->write($tasks);
		$jsonFile->close();
	}
}

/**
 * A module class that implements the default methods defined by the
 * OMVIModuleService interface.
 * @ingroup api
 */
abstract class OMVModuleServiceAbstract extends OMVModuleAbstract
  implements OMVIModuleService {
	/**
	 * Defines the modules that if present should build their configuration
	 * before the service provided by this module.
	 * @return An array of modules.
	 */
	public function shouldApply() { return array(); }

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return array(); }

	/**
	 * Defines the modules that if present should start after the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	function startBefore() { return array(); }

	/**
	 * Defines the modules that if present should be stopped after the
	 * service provided by this module.
	 * @return An array of modules.
	 */
	function shouldStop() { return array(); }

	/**
	 * Defines the modules that if present should stop before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	function stopAfter() { return array(); }

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {}

	/**
	 * Start the managed service.
	 */
	public function startService() {}
}

/**
 * Interface to implement a service managed by the module.
 * @ingroup api
 */
interface OMVIModuleService {
	/**
	 * Defines the modules that if present should build their configuration
	 * before the service provided by this module. Nevertheless, the module
	 * can still apply the configuration if the listed dependencies are
	 * missing.
	 * @return An array of modules. Returns an empty array if no
	 *   dependencies exist.
	 */
	function shouldApply();

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module. Nevertheless, the module can still start if
	 * the listed dependencies are missing.
	 * @return An array of modules. Returns an empty array if no
	 *   dependencies exist.
	 */
	function shouldStart();

	/**
	 * Defines the modules that if present should start after the service
	 * provided by this module.
	 * @return An array of modules. Returns an empty array if no
	 *   dependencies exist.
	 */
	function startBefore();

	/**
	 * Defines the modules that if present should be stopped after the
	 * service provided by this module. Normally you would include here the
	 * same dependencies as those provided with the shouldStart method.
	 * Nevertheless, the module can still stop if the listed dependencies
	 * are missing.
	 * @return An array of modules. Returns an empty array if no
	 *   dependencies exist.
	 */
	function shouldStop();

	/**
	 * Defines the modules that if present should stop before the service
	 * provided by this module.
	 * @return An array of modules. Returns an empty array if no
	 *   dependencies exist.
	 */
	function stopAfter();

	/**
	 * Generate the configuration.
	 */
	function applyConfig();

	/**
	 * Stop the managed service.
	 */
	function stopService();

	/**
	 * Start the managed service.
	 */
	function startService();
}

/**
 * The module manager.
 * @ingroup api
 */
class OMVModuleMgr extends OMVObject {
	private $map = array();

	/**
	 * Returns a module manager singleton.
	 * @return The module manager object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if(!isset($instance))
			$instance = new OMVModuleMgr();
		return $instance;
	}

	/**
	 * Register a module.
	 * @param module The module class to be registered.
	 * @return Returns TRUE on success or FALSE on failure.
	 * @throw E_MISC_FAILURE
	 */
	final public function registerModule(OMVModuleAbstract $module) {
		if(!isset($module))
			return FALSE;
		$name = $module->getName();
		// Check if module already exists.
		if(FALSE !== $this->getModule($name)) {
//			$this->debug("Module '{$name}' already registered");
			throw new OMVException(
			  OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Module already registered (name=%s, class=%s)",
			  $name, get_class($module)));
		}
		$this->map[strtolower($name)] = $module;
		ksort($this->map);
		return TRUE;
	}

	/**
	 * Get a module.
	 * @param name The name of the module to get.
	 * @return The module object or FALSE on failure.
	 */
	final public function getModule($name) {
		$name = strtolower($name);
		if(!array_key_exists($name, $this->map))
			return FALSE;
		return $this->map[$name];
	}

	/**
	 * Get all registered modules.
	 * @return An array containing all registered modules.
	 */
	final public function getModules() {
		return $this->map;
	}

	/**
	 * Helper function to get a list of dirty modules.
	 * @return An array containing the names of the dirty marked modules.
	 */
	final public function getDirtyModules() {
		$jsonFile = new OMVJsonFile($GLOBALS['OMV_ENGINED_DIRTY_MODULES_FILE']);
		if(!$jsonFile->exists())
			return array();
		$jsonFile->open("r", LOCK_SH);
		$modules = array_unique($jsonFile->read());
		$jsonFile->close();
		return $modules;
	}

	/**
	 * Helper function to mark a module as dirty.
	 * @param name The name of the module.
	 * @return The list of dirty modules.
	 */
	final public function setModuleDirty($name) {
		$jsonFile = new OMVJsonFile($GLOBALS['OMV_ENGINED_DIRTY_MODULES_FILE']);
		$jsonFile->open("c+");
		$modules = $jsonFile->read();
		$modules[] = strtolower($name);
		$modules = array_unique($modules);
		$jsonFile->write($modules);
		$jsonFile->close();
		return $modules;
	}

	/**
	 * Helper function to check whether a module is marked dirty.
	 * @param name The name of the module.
	 * @return TRUE if the module is marked dirty, otherwise FALSE.
	 */
	final public function isModuleDirty($name) {
		$modules = $this->getDirtyModules();
		return in_array(strtolower($name), $modules);
	}

	/**
	 * Dump all registered modules.
	 */
	final public function dump() {
		print("Registered modules:\n");
		foreach($this->map as $modulek => $modulev) {
			printf("  %s\n", $modulek);
		}
	}
}
?>
