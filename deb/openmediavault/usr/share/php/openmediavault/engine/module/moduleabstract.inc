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
 * The core module class.
 * @ingroup api
 */
abstract class ModuleAbstract {
	use \OMV\DebugTrait;

	/**
	 * Get the name of the module.
	 * @return string The name of the module.
	 */
	abstract public function getName();

	/**
	 * Get the alias of the module.
	 * @return string|null The alias of the module. Defaults to NULL.
	 */
	public function getAlias() {
		return NULL;
	}

	/**
	 * Get a brief description of the module and its function.
	 * @return string The description of the module. Defaults
	 *   to an empty string.
	 */
	public function getDescription() {
		return "";
	}

	/**
	 * Helper function to mark this module as dirty.
	 * @return array The list of dirty modules.
	 */
	final public function setDirty() {
		return $this->setDirtyByName($this->getName());
	}

	/**
	 * Helper function to mark the given module as dirty.
	 * @param name The name of the module.
	 * @return array The list of dirty modules.
	 */
	final public function setDirtyByName($name) {
		$moduleMngr = Manager::getInstance();
		return $moduleMngr->setModuleDirty($name);
	}

	/**
	 * Helper function to check whether the module is marked dirty.
	 * @return bool TRUE if the module is marked dirty, otherwise FALSE.
	 */
	final public function isDirty() {
		$moduleMngr = Manager::getInstance();
		return $moduleMngr->isModuleDirty($this->getName());
	}

	/**
	 * Helper function to execute an external program.
	 * @param command The command that will be executed.
	 * @param output If the output argument is present, then the specified
	 *   array will be filled with every line of output from the command.
	 *   Trailing whitespace, such as \n, is not included in this array.
	 * @return integer The exit code of the command.
	 */
	final protected function exec($command, &$output = NULL) {
		$cmd = new \OMV\System\Process($command);
		$cmd->setRedirect2to1();
		$cmd->execute($output, $exitStatus);
		return $exitStatus;
	}

	/**
	 * Helper function to get the administrator context.
	 * @return array The administrator context object.
	 */
	final protected function getAdminContext() {
		return [
			"username" => "admin",
			"role" => OMV_ROLE_ADMINISTRATOR
		];
	}

	/**
	 * Add a task that can be performed at a later date.
	 * @param id The identifier of the task type.
	 * @param name The name of the class method to be executed.
	 * @param params The parameters passed to the method when it is executed.
	 */
	final protected function addTask($id, $name, $params) {
		// Check if class method exists.
		if (!method_exists($this, $name)) {
			throw new \OMV\Exception(
			  "The method '%s' does not exist for module '%s'.",
			  $name, $this->getName());
		}
		// Add new task to the modules pending task list. Append
		// the new task if the file already exists, otherwise
		// create a new file.
		$filename = sprintf("/var/lib/openmediavault/%s_tasks.json",
		  $this->getName());
		$jsonFile = new \OMV\Json\File($filename);
		$jsonFile->open("c+");
		// Make sure we have a valid JSON file. Keep in mind that the file
		// is empty if it was created by the \OMV\Json\File::open method.
		if ($jsonFile->isEmpty())
			$jsonFile->write([]);
		// Read the file content.
		$tasks = $jsonFile->read();
		// Add the new task to the list of pending tasks.
		$tasks[] = [
			"id" => $id,
			"func" => $name,
			"params" => $params
		];
		// Write the pending task list.
		$jsonFile->write($tasks);
		$jsonFile->close();
	}

	/**
	 * Execute the tasks with the given identifier.
	 * @param id The identifier of the tasks to be executed.
	 */
	final protected function execTasks($id) {
		$filename = sprintf("/var/lib/openmediavault/%s_tasks.json",
			$this->getName());
		$jsonFile = new \OMV\Json\File($filename);
		if (!$jsonFile->exists())
			return;
		$jsonFile->open("c+");
		$tasks = $jsonFile->read();
		// Get list of tasks to be executed.
		$runTasks = array_filter_ex($tasks, "id", $id);
		if (is_null($runTasks))
			return;
		foreach ($runTasks as $taskk => $taskv) {
			$this->debug("Executing task (callback=%s::%s)",
				get_class($this), $taskv['func']);
			call_user_func_array(array($this, $taskv['func']), [
				$taskv['params']
			]);
		}
		// Remove executed tasks from list. Note, the array must be
		// re-indexed.
		$tasks = array_values(array_diff_key($tasks, $runTasks));
		// Write back list of not executed task.
		$jsonFile->write($tasks);
		$jsonFile->close();
	}
}
