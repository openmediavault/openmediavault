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
	 * @param id The data model identifier. If \em id is not set, then
	 *   the xpath will be used as identifier instead.
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
	 * @return TRUE if the object is referenced, otherwise FALSE.
	 */
	public function isReferenced(ConfigObject $object) {
		if (FALSE === $object->isReferenceable()) {
			throw new DatabaseException(
			  "The configuration object can not be referenced.");
		}
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->buildIsReferencedQuery($object);
		return $this->getBackend()->exists($xpath);
	}

	public function exists($id, $property, $value) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildExistsQuery($property, $value);
		return $this->getBackend()->exists($xpath);
	}
}
