<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
namespace OMV\DataModel;

require_once("openmediavault/object.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/json/file.inc");
require_once("openmediavault/datamodel/model.inc");

/**
 * The data model manager keeps track of all model types.
 */
class Manager extends \OMVObject {
	private $map = array();

	/**
	 * Returns the data model manager singleton.
	 * @return The singleton object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new Manager();
		return $instance;
	}

	/**
	 * Load the data models.
	 * @return Returns TRUE on success or FALSE on failure.
	 */
	final public function load($path) {
		foreach (new \DirectoryIterator($path) as $item) {
			if ($item->isDot())
				continue;
			if (!$item->isFile())
				continue;
			if ("json" !== strtolower($item->getExtension()))
				continue;
			$file = new \OMV\Json\File($item->getPathname());
			if (!$file->exists())
				continue;
			$file->open("r");
			$schema = $file->read();
			$file->close();
			$model = new Model($schema);
			if (!$this->registerModel($model))
				return FALSE;
		}
		return TRUE;
	}

	/**
	 * Register a model.
	 * @param model The model class to be registered.
	 * @return Returns TRUE on success or FALSE on failure.
	 */
	final public function registerModel(Model $model) {
		if (!isset($model))
			return FALSE;
		$ids = array();
		$ids[] = $model->getId();
		$ids[] = $model->getAlias();
		foreach ($ids as $idk => $idv) {
			if (empty($idv))
				continue;
			// Check if the model already exists.
			if (FALSE !== $this->getModel($idv)) {
				$this->setLastError(
				  sprintf("The data model '%s' is already registered.",
				  $idv));
				return FALSE;
			}
			// Append the model.
			$this->map[strtolower($idv)] = $model;
		}
		return TRUE;
	}

	/**
	 * Get a model.
	 * @param id The id of the model to get.
	 * @return The model object or FALSE on failure.
	 */
	final public function getModel($id) {
		$id = strtolower($id);
		if (!array_key_exists($id, $this->map))
			return FALSE;
		return $this->map[$id];
	}

	/**
	 * Get all registered models.
	 * @return An array containing all registered models.
	 */
	final public function getModels() {
		return $this->map;
	}

	/**
	 * Dump all registered models.
	 */
	final public function dump() {
		print("Registered data models:\n");
		foreach ($this->getModels() as $modelk => $modelv) {
			printf("  %s\n", $modelk);
		}
	}
}
