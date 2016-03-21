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
	 * Get the specified configuration object. The method can be called
	 * the following way:
	 * id [[arg0] [arg1] [arg2] ...]
	 * If this method is called only with \em id, then non iterable
	 * configuration objects are assumed, e.g. "conf.service.ftp".
	 * Example: xxx->get("conf.service.ftp")
	 * If only arg0 is given, it is assumed to be the UUID of the
	 * configuration object.
	 * Example: xxx->get("conf.service.ftp.share", "f819afcc-eda1-...")
	 * If you call this method with multiple args, then arg0 is used
	 * as the property and the following args as values. Multiple
	 * values are combined via OR.
	 * Example: xxx->get("conf.system.hdparm", "devicefile",
	 *   "/dev/sdc", "/dev/disk/by-id/wwn-0x500253855031cfa5")
	 * @param id The data model identifier.
	 * @param ...args Additional arguments, e.g. the object UUID.
	 * @return The configuration object(s).
	 * @throw \OMV\Config\DatabaseException
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
		if ((TRUE === $model->isIterable()) && (FALSE === $hasArgs))
			$data = $this->getBackend()->getList($xpath);
		else
			$data = $this->getBackend()->get($xpath);
		if (is_null($data) || (is_array($data) && empty($data))) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
			  $xpath);
		}
		if ((TRUE === $model->isIterable()) && (FALSE === $hasArgs)) {
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

	public function getAssoc($id, ...$args) {
		$objects = $this->get($id, ...$args);
		if (is_array($objects)) {
			$result = array();
			foreach ($objects as $objectk => $objectv) {
				$result[] = $objectv->getAssoc();
			}
		} else {
			$result = $objects->getAssoc();
		}
		return $result;
	}

	/**
	 * Get iterable configuration objects that are matching the specified
	 * predicate.
	 * @param id The data model identifier.
	 * @param predicate The predicate to search for, e.g. <ul>
	 *   \li "not(contains(opts,'bind'))"
	 *   \li "devicefile='/dev/sda' or devicefile='/dev/disk/by-id/ata-ST1000DM003-1CH132_S2DF80PC']"
	 *   </ul>
	 *   The predicate must be defined in XPath syntax.
	 * @return An array of configuration objects.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function getByPredicate($id, $predicate) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Is the configuration object iterable?
		if (FALSE === $model->isIterable()) {
			throw new DatabaseException(
			  "The configuration object is not iterable.");
		}
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildGetByPredicateQuery($predicate);
		// Redirect the query to the database backend.
		$data = $this->getBackend()->getList($xpath);
		// Create the configuration objects.
		$result = array();
		foreach ($data as $datak => $datav) {
			$object = new ConfigObject($id);
			$object->setAssoc($datav, FALSE);
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Set the configuration object at the specified XPath expression.
	 * If the configuration object is iterable and identified as new,
	 * then the identifier property (in most cases 'uuid') will be
	 * generated and set automatically.
	 * @param object The configuration object to use.
	 * @return None.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function set(ConfigObject &$object) {
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		// Build the XPath query string.
		$xpath = $queryBuilder->buildSetQuery($object);
		if ((TRUE === $object->isIterable()) && (TRUE === $object->isNew())) {
			// Auto-create the identifier.
			$object->createIdentifier();
			// The values to be stored must look a little bit different
			// in this case.
			// XPath in query info: //services/ftp/shares/share
			// XPath build by the query builder: //services/ftp/shares
			// Values to put: array(
			//    "share" => array(...)
			// )
			$qi = $object->getModel()->getQueryInfo();
			$parts = explode("/", $qi['xpath']);
			$nodeName = array_pop($parts);
			$values = array($nodeName => $object->getAssoc());
			// Finally put the configuration object.
			$result = $this->getBackend()->set($xpath, $values);
		} else {
			// Get the values of the configuration object.
			$values = $object->getAssoc();
			// Finally put the configuration object.
			$result = $this->getBackend()->replace($xpath, $values);
		}
		if (FALSE === $result) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
			  $xpath);
		}
	}

	/**
	 * Delete the nodes matching the specified XPath expression.
	 * @param object The configuration object to use.
	 * @return None.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function delete(ConfigObject $object) {
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->buildDeleteQuery($object);
		if (FALSE === $this->getBackend()->delete($xpath)) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
			  $xpath);
		}
	}

	/**
	 * Delete a configuration object with the specified predicate.
	 * @param id The data model identifier.
	 * @param predicate The predicate to search for, e.g. <ul>
	 *   \li "not(contains(opts,'bind') or contains(opts,'loop'))"
	 *   \li "devicefile='/dev/sda' or devicefile='/dev/disk/by-id/ata-ST1000DM003-1CH132_S2DF80PC']"
	 *   </ul>
	 *   The predicate must be defined in XPath syntax.
	 * @return None.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function deleteByPredicate($id, $predicate) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildGetByPredicateQuery($predicate);
		if (FALSE === $this->getBackend()->delete($xpath)) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
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
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->buildIsReferencedQuery($object);
		return $this->getBackend()->exists($xpath);
	}

	/**
	 * Assert that the specified configuration object is referenced.
	 * @param object The configuration object to use.
	 * @return None.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function assertIsReferenced(ConfigObject $object) {
		if (FALSE === $this->isReferenced($object)) {
			throw new DatabaseException(
			  "The configuration object is not referenced.");
		}
	}

	/**
	 * Assert that the specified configuration object is not referenced.
	 * @param object The configuration object to use.
	 * @return None.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function assertIsNotReferenced(ConfigObject $object) {
		if (TRUE === $this->isReferenced($object)) {
			throw new DatabaseException(
			  "The configuration object is referenced.");
		}
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
	 * Check if a configuration object with the specified predicate
	 * exists.
	 * @param id The data model identifier.
	 * @param predicate The predicate to search for, e.g. <ul>
	 *   \li "not(contains(opts,'bind') or contains(opts,'loop'))"
	 *   \li "devicefile='/dev/sda' or devicefile='/dev/disk/by-id/ata-ST1000DM003-1CH132_S2DF80PC']"
	 *   </ul>
	 *   The predicate must be defined in XPath syntax.
	 * @return TRUE if data exists, otherwise FALSE.
	 */
	public function existsByPredicate($id, $predicate) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildGetByPredicateQuery($predicate);
		return $this->getBackend()->exists($xpath);
	}

	/**
	 * Check if a configuration object with the value of the specified
	 * property is unique.
	 * @param object The configuration object to use.
	 * @param property The name of the data model property.
	 * @return TRUE if no configuration object with the same property
	 *   value exists, otherwise FALSE.
	 */
	public function isUnique(ConfigObject $object, $property) {
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->buildExistsQuery($property,
		  $object->get($property));
		return !$this->getBackend()->exists($xpath);
	}

	/**
	 * Assert that a configuration object with the value of the specified
	 * property is unique.
	 * @param object The configuration object to use.
	 * @return None.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function assertIsUnique(ConfigObject $object, $property) {
		if (FALSE === $this->isUnique($object, $property)) {
			throw new DatabaseException("The configuration object is not ".
			  "unique. An object with the property '%s' and value '%s'".
			  "already exists.", $property, $object->get($property));
		}
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
