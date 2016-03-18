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
namespace OMV\Config;

require_once("openmediavault/globals.inc");

class Database {
	private $backend = NULL;

	public function __construct() {
		$this->backend = new DatabaseBackend($GLOBALS['OMV_CONFIG_FILE']);
		$this->backend->setVersioning(TRUE);
		$this->backend->load();
	}

	/**
	 * Returns the configuration database singleton.
	 * @return The singleton object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new Database();
		return $instance;
	}

	public function getBackend() {
		return $this->backend;
	}

	/**
	 * Get the configuration object at the specified XPath expression.
	 * @param id The data model identifier.
	 * @param ...args Additional arguments, e.g. the object UUID.
	 * @return The configuration object.
	 */
	public function get($id, ...$args) {
		// Check if we have to add additional arguments to the query.
		// In most cases this is the UUID of the requested configuration
		// object.
		$hasArgs = (0 < count($args));
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildGetQuery(...$args);
		// Redirect the query to the database backend.
		if ($model->isIterable() && (FALSE === $hasArgs))
			$data = $this->getBackend()->getList($xpath);
		else
			$data = $this->getBackend()->get($xpath);
		if (is_null($data) || (is_array($data) && empty($data))) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
		if ($model->isIterable() && (FALSE === $hasArgs)) {
			$result = array();
			foreach ($data as $datak => $datav) {
				$object = new ConfigObject($id);
				$object->setAssoc($datav, FALSE);
				$result[] = $object;
			}
		} else {
			$result = new ConfigObject($id);
			$result->setAssoc($data, FALSE);
		}
		return $result;
	}

	/**
	 * Set the configuration object at the specified XPath expression.
	 * @param object The configuration object to use.
	 * @return None.
	 */
	public function set(ConfigObject $object) {
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->buildSetQuery($object);
		if ((TRUE === $object->isIterable()) && (TRUE === $object->isNew())) {
			$success = $this->getBackend()->set($xpath,
			  $queryBuilder->getValues($object));
		} else {
			$success = $this->getBackend()->replace($xpath,
			  $queryBuilder->getValues($object));
		}
		if (FALSE === $success) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
	}

	/**
	 * Delete the nodes matching the specified XPath expression.
	 * @param object The configuration object to use.
	 * @return None.
	 */
	public function delete(ConfigObject $object) {
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->buildDeleteQuery($object);
		if (FALSE === $this->getBackend()->delete($xpath)) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
	}

	/**
	 * Check if the specified object is referenced.
	 * @param object The configuration object to use.
	 * @param quiet Set to FALSE to do throw an exception. Defaults to TRUE.
	 * @return TRUE if the object is referenced, otherwise FALSE.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function isReferenced(ConfigObject $object, $quiet = TRUE) {
		if (FALSE === $object->isReferenceable()) {
			throw new DatabaseException(
			  "The configuration object can not be referenced.");
		}
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->buildIsReferencedQuery($object);
		$result = $this->getBackend()->exists($xpath);
		if ((FALSE === $quiet) && (TRUE === $result)) {
			throw new DatabaseException(
			  "The configuration object is referenced.");
		}
		return $result;
	}

	/**
	 * Check if a configuration object with the specified arguments
	 * exists.
	 * @param id The data model identifier.
	 * @param property The name of the data model property.
	 * @param value The value to be searched. This can be also an array
	 *   of values.
	 * @return TRUE if data exists, otherwise FALSE.
	 */
	public function exists($id, $property, $value) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		if (!is_array($value))
			$value = array($value);
		foreach ($value as $valuek => $valuev) {
			$xpath = $queryBuilder->buildExistsQuery($property, $value);
			if (TRUE === $this->getBackend()->exists($xpath))
				return TRUE;
		}
		return FALSE;
	}

	/**
	 * Check if a configuration object with the value of the specified property
	 * already exists.
	 * @param object The configuration object to use.
	 * @param property The name of the data model property.
	 * @param quiet Set to FALSE to do throw an exception. Defaults to TRUE.
	 * @return TRUE if no configuration object with the same property
	 *   value exists, otherwise FALSE.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function isUnique(ConfigObject $object, $property, $quiet = TRUE) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildExistsQuery($property,
		  $object->get($property));
		$result = $this->getBackend()->exists($xpath);
		if ((FALSE === $quiet) && (TRUE === $result)) {
			throw new DatabaseException("The configuration object is not ".
			  "unique (property='%s', value='%s').", $property,
			  $object->get($property));
		}
		return !$result;
	}

	/**
	 * Unlink all revision files.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function unlinkRevisions() {
		return $this->getBackend()->unlinkRevisions();
	}

	/**
	 * Revert changes. All existing revision files will be deleted.
	 * @param filename The revision file. Defaults to NONE.
	 * @return None.
	 */
	public function revert($filename) {
		$backend = $this->getBackend();
		if (FALSE === $backend->revert($filename)) {
			throw new DatabaseException(
			  "Failed to revert configuration changes: %s",
		  	  $backend->getLastError());
		}
	}
}
