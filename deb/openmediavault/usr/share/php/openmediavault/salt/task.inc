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
namespace OMV\Salt;

/**
 * @ingroup api
 */
class Task {
	/**
	 * Helper function to create a Salt task that will be executed when the
	 * configuration changes are applied (see RPC Config::applyChanges).
	 * @param string sls The name of the Salt state.
	 * @param mixed pillar The pillar data to be used.
	 */
	final public static function create($sls, $pillar) {
		$tasks = [];
		$filename = \OMV\Environment::get("OMV_SALT_TASKS_FILE",
			"/srv/pillar/omv/tasks.sls");
		// Open the file and load the content if it exists.
		if (file_exists($filename))
			$tasks = yaml_parse_file($filename);
		// Make sure the data is valid and the key 'tasks' exists.
		if (!array_key_exists("tasks", $tasks))
			$tasks['tasks'] = [];
		// Append a new task.
		$tasks['tasks'][] = [
			"sls" => $sls,
			"pillar" => $pillar
		];
		// Convert the PHP dictionary into YAML. We need to
		// remove the first and last line which looks like:
		// ---
		// tasks:
		//   sls: xxx
		//   pillar: xxx
		// ...
		$tasks = yaml_emit($tasks, YAML_UTF8_ENCODING, YAML_LN_BREAK);
		$tasks = explode("\n", $tasks);
		while ("---" !== array_shift($tasks));
		while ("..." !== array_pop($tasks));
		$tasks = implode("\n", $tasks);
		// Update the pillar file.
		file_put_contents($filename, $tasks, LOCK_EX);
		// Finally mark the 'Task' module as dirty.
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		return $moduleMngr->setModuleDirty("task");
	}
}
