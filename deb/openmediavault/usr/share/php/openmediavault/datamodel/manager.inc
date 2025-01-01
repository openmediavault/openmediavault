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
namespace OMV\DataModel;

require_once("openmediavault/functions.inc");

/**
 * The data model manager keeps track of all model types.
 */
class Manager {
	private $map = [];
	private $loaded = false;

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

	final public function isLoaded() {
		return $this->loaded;
	}

	/**
	 * Load the data models.
	 * @return void
	 */
	final public function load() {
		if ($this->isLoaded())
			return;
		foreach (listdir(\OMV\Environment::get("OMV_DATAMODELS_DIR"),
		  "json") as $path) {
			// Read the JSON content and decode it.
			$file = new \OMV\Json\File($path);
			if (!$file->exists())
				continue;
			$file->open("r");
			$jsonModel = $file->read();
			$file->close();
			// Note, a file can contain multiple model definitions.
			if (is_assoc_array($jsonModel))
				$jsonModel = array($jsonModel);
			foreach ($jsonModel as $jsonModelk => $jsonModelv) {
				// Validate the data model definition.
				$schema = new \OMV\Json\Schema('{
					"type":"object",
					"properties":{
						"type":{"type":"string","enum":["rpc","config"],"required":true},
						"id":{"type":"string","required":true},
						"alias":{"type":"string","required":false},
						"title":{"type":"string","required":false},
						"description":{"type":"string","required":false}
					}
				}');
				try {
					// Convert the associative array into a JSON string
					// before validating the model definition.
					$schema->validate(json_encode_safe($jsonModelv));
				} catch(\OMV\Json\SchemaValidationException $e) {
					throw new \OMV\Exception(
					  "The data model definition in file '%s' is invalid: %s",
					  $path, $e->getMessage());
				}
				// Create the data model.
				switch ($jsonModelv['type']) {
				case "rpc":
					$model = new \OMV\Rpc\DataModel($jsonModelv);
					break;
				case "config":
					$model = new \OMV\Config\DataModel($jsonModelv);
					break;
				default:
					throw new \OMV\Exception(
					  "Unknown data model type '%s' in file '%s'.",
					  $jsonModelv['type'], $path);
					break;
				}
				$this->registerModel($model);
			}
		}
		$this->loaded = true;
	}

	/**
	 * Register a model.
	 * @param model The model class to be registered.
	 * @return void
	 */
	final public function registerModel(DataModel $model) {
		$ids = [];
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
			$this->map[mb_strtolower($idv)] = $model;
			ksort($this->map);
		}
	}

	/**
	 * Check if a data model is registered.
	 * @return TRUE if the data model is already registered, otherwise FALSE.
	 */
	final public function isRegistered($id) {
		return array_key_exists(mb_strtolower($id), $this->map);
	}

	/**
	 * Get a model.
	 * @param id The id of the data model to get.
	 * @return The data model object.
	 * @throw \OMV\Exception
	 */
	final public function getModel($id) {
		$id = mb_strtolower($id);
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
