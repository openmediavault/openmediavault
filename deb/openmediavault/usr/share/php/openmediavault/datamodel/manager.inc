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

/**
 * The data model manager keeps track of all model types.
 */
class Manager {
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
	 * @param directory The directory that contains the data model schemas.
	 * @return None.
	 */
	final public function load($directory) {
		foreach (listdir($directory, "json") as $path) {
			// Read the JSON content and decode it.
			$file = new \OMV\Json\File($path);
			if (!$file->exists())
				continue;
			$file->open("r");
			$schema = $file->read();
			$file->close();
			// Note, a schema file can contain multiple definitions.
			if (is_assoc($schema))
				$schema = array($schema);
			foreach ($schema as $schemak => $schemav) {
				$model = new Model($schemav);
				$this->registerModel($model);
			}
		}
	}

	/**
	 * Register a model.
	 * @param model The model class to be registered.
	 * @return None.
	 */
	final public function registerModel(Model $model) {
		// Validate the data model definition.
		$schema = new \OMV\Json\Schema('{
			"type":"object",
			"properties":{
				"type":"object",
				"properties":{
					"type":{"type":"string","enum":["object"],"required":true},
					"id":{"type":"string","required":true},
					"alias":{"type":"string","required":false},
					"title":{"type":"string","required":false},
					"properties":{"type":"object","required":true}
				}
			}
		}');
		try {
			$schema->validate($model->getSchema());
		} catch(\OMV\Json\SchemaValidationException $e) {
			throw new \OMV\Exception(
			  "The data model definition is invalid: %s",
			  $e->getMessage());
		}
		$ids = array();
		$ids[] = $model->getId();
		$ids[] = $model->getAlias();
		foreach ($ids as $idk => $idv) {
			if (empty($idv))
				continue;
			// Check if the model already exists.
			if (TRUE === $this->isRegistered($idv)) {
				throw new \OMV\Exception(
				  "The data model '%s' is already registered.",
				  $idv);
			}
			// Append the model.
			$this->map[strtolower($idv)] = $model;
			ksort($this->map);
		}
	}

	/**
	 * Check if a data model is registered.
	 * @return TRUE if the data model is already registered, otherwise FALSE.
	 */
	final public function isRegistered($id) {
		return array_key_exists(strtolower($id), $this->map);
	}

	/**
	 * Get a model.
	 * @param id The id of the data model to get.
	 * @return The data model object.
	 * @throw \OMV\Exception
	 */
	final public function getModel($id) {
		$id = strtolower($id);
		if (!array_key_exists($id, $this->map)) {
			throw new \OMV\Exception(
			  "The data model '%s' does not exist.",
			  $id);
		}
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
