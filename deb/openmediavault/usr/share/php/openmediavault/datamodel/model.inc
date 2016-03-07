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

class Model extends \OMVObject {
	private $schema = NULL;

	/**
	 * @param schema The schema as in JSON schema syntax.
	 */
	public function __construct($schema) {
		if (!($schema instanceof Schema))
			$schema = new Schema($schema);
		$this->schema = $schema;
	}

	public function getSchema() {
		return $this->schema;
	}

	public function getId() {
		$schemaAssoc = $this->getSchema()->getAssoc();
		return array_value($schemaAssoc, "id", FALSE);
	}

	public function getAlias() {
		$schemaAssoc = $this->getSchema()->getAssoc();
		return array_value($schemaAssoc, "alias", FALSE);
	}

	public function getTitle() {
		$schemaAssoc = $this->getSchema()->getAssoc();
		return array_value($schemaAssoc, "title", "");
	}

	public function getDescription() {
		$schemaAssoc = $this->getSchema()->getAssoc();
		return array_value($schemaAssoc, "description", "");
	}

	public function validate($value) {
		$schema = $this->getSchema();
		if (FALSE === $schema->validate($value)) {
			$this->setLastError($schema->getLastError());
			return FALSE;
		}
		return TRUE;
	}

	public function validateProperty($name, $value) {
		$schema = $this->getSchema();
		if (FALSE === $schema->validate($value, $name)) {
			$this->setLastError($schema->getLastError());
			return FALSE;
		}
		return TRUE;
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
		if (FALSE === $this->getSchema()->getAssocByPath($name)) {
			$this->setLastError(
			  sprintf("The property '%s' does not exist.", $name));
			return FALSE;
		}
		return TRUE;
	}

	public function walk($callback, &$userData = NULL, $schema = NULL) {
		if (NULL == $schema)
			$schema = $this->getSchema()->getAssoc();
		if (array_key_exists("type", $schema) && ("object" == $schema['type'])) {
			call_user_func_array($callback, array("", $schema, &$userData));
			if (!array_key_exists("properties", $schema))
				return;
			foreach ($schema['properties'] as $propName => $propSchema) {
				call_user_func_array($callback, array($propName,
				  $propSchema, &$userData));
				$this->walk($callback, $userData, $propSchema);
			}
		}
	}
}
