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
require_once("openmediavault/object.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/jsonschema.inc");

class OMVDataModelSchema extends OMVJsonSchema {
	final public function getTypeByPath($name) {
		if (FALSE === ($schema = $this->getSchemaAssocByPath($name)))
			return FALSE;
		return $schema['type'];
	}

	/**
	 * Add some more formats than JSON schema supports.
	 */
	protected function checkFormat($value, $schema, $name) {
		if (TRUE === parent::checkFormat($value, $schema, $name))
			return TRUE;
		$this->clearLastError();
		$valid = TRUE;
		switch ($schema['format']) {
		case "ipv4":
			if (!filter_var($value, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not an IPv4 address.",
				  $name, $value));
			}
			break;
		case "uuidv4":
			if (!is_uuid($value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not an UUIDv4.",
				  $name, $value));
			}
			break;
		case "fsuuid":
			if (!is_fs_uuid($value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not a valid filesystem UUID.",
				  $name, $value));
			}
			break;
		default:
			$valid = FALSE;
			$this->setLastError(
			  sprintf("%s: The format '%s' is not defined.",
			  $name, $schema['format']));
			break;
		}
		return $valid;
	}
}

class OMVDataModel extends OMVObject {
	private $schema = NULL;

	/**
	 * @param schema The schema as in JSON schema syntax.
	 */
	public function __construct($schema) {
		if (!($schema instanceof OMVDataModelSchema))
			$schema = new OMVDataModelSchema($schema);
		$this->schema = $schema;
	}

	public function getSchema() {
		return $this->schema;
	}

	public function getId() {
		$schemaAssoc = $this->getSchema()->getSchemaAssoc();
		return array_value($schemaAssoc, "id", FALSE);
	}

	public function getAlias() {
		$schemaAssoc = $this->getSchema()->getSchemaAssoc();
		return array_value($schemaAssoc, "alias", FALSE);
	}

	public function getTitle() {
		$schemaAssoc = $this->getSchema()->getSchemaAssoc();
		return array_value($schemaAssoc, "title", "");
	}

	public function getDescription() {
		$schemaAssoc = $this->getSchema()->getSchemaAssoc();
		return array_value($schemaAssoc, "description", "");
	}

	public function validateProperty($name, $value) {
		return $this->getSchema()->validate($value, $name);
	}

	public function convertProperty($name, $value) {
		if (FALSE === ($type = $this->getSchema()->getTypeByPath($name)))
			return $value;
		switch ($type) {
		case "boolean":
			$value = boolvalEx($value);
			break;
		case "integer":
			$value = intval($value);
			break;
		case "double":
		case "number":
			$value = doubleval($value);
			break;
		case "string":
			$value = strval($value);
			break;
		}
		return $value;
	}

	public function propertyExists($name) {
		if (FALSE === $this->getSchema()->getSchemaAssocByPath($name))
			return FALSE;
		return TRUE;
	}

	public function getDefaultAssoc() {
		$schemaAssoc = $this->getSchema()->getSchemaAssoc();
		$object = array();
		$this->_getDefaultAssoc($schemaAssoc, $object);
		return $object;
	}

	/**
	 * Helper method.
	 */
	private function _getDefaultAssoc($schema, &$object) {
		if (array_key_exists("type", $schema) && ("object" == $schema['type'])) {
			$object = array();
			if (!array_key_exists("properties", $schema))
				return;
			foreach ($schema['properties'] as $propName => $propSchema) {
				$default = NULL;
				if (array_key_exists("default", $propSchema))
					$default = $propSchema['default'];
				$object[$propName] = $default;
				$this->_getDefaultAssoc($propSchema, $object[$propName]);
			}
		}
	}
}

/**
 * The data model manager keeps track of all OMVDataModel types.
 */
class OMVDataModelManager extends OMVObject {
	private $map = array();

	/**
	 * Returns the data model manager singleton.
	 * @return The singleton object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new OMVDataModelManager();
		return $instance;
	}

	/**
	 * Load the data models.
	 * @return Returns TRUE on success or FALSE on failure.
	 */
	final public function load($path) {
		foreach (new DirectoryIterator($path) as $item) {
			if ($item->isDot())
				continue;
			if (!$item->isFile())
				continue;
			if ("json" !== strtolower($item->getExtension()))
				continue;
			$jsonFile = new OMVJsonFile($item->getPathname());
			if (!$jsonFile->exists())
				continue;
			$jsonFile->open("r");
			$schema = $jsonFile->read();
			$jsonFile->close();
			$model = new OMVDataModel($schema);
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
	final public function registerModel(OMVDataModel $model) {
		if (!isset($model))
			return FALSE;
		$ids = array();
		$ids[] = $model->getId();
		$ids[] = $model->getAlias();
		foreach ($ids as $idk => $idv) {
			// Check if the model already exists.
			if (FALSE !== $this->getModel($idv)) {
				$this->setLastError(
				  sprintf("The data model is already registered (id=%s).",
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
