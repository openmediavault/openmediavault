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
	 * Get the specified configuration object.
	 * @param id The data model identifier, e.g. 'conf.service.ftp'.
	 * @param uuid The UUID of an configuration object. Defaults to NULL.
	 * @return Depending on the configuration object and whether \em uuid
	 *   is set, an array of configuration objects or a single object is
	 *   returned.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function get($id, $uuid = NULL) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildGetQuery($uuid);
		// Redirect the query to the database backend.
		if ((TRUE === $model->isIterable()) && is_null($uuid))
			$data = $this->getBackend()->getList($xpath);
		else
			$data = $this->getBackend()->get($xpath);
		if (is_null($data)) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
			  $xpath);
		}
		if ((TRUE === $model->isIterable()) && is_null($uuid)) {
			$result = [];
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

	public function getAssoc($id, $uuid = NULL) {
		$objects = $this->get($id, $uuid);
		if (is_array($objects)) {
			$result = [];
			foreach ($objects as $objectk => $objectv) {
				$result[] = $objectv->getAssoc();
			}
		} else {
			$result = $objects->getAssoc();
		}
		return $result;
	}

	/**
	 * Get the iterable configuration objects that are matching the specified
	 * constraints.
	 * @param id The data model identifier.
	 * @param filter A filter specifying constraints on the objects
	 *   to retrieve.
	 *   @code
	 *   Example 1:
	 *   [
	 *       "operator" => "stringEquals",
	 *       "arg0" => "fsname",
	 *       "arg1" => $params['id']
	 *   ]
	 *
	 *   Example 2:
	 *   [
	 *       "operator": "and",
	 *       "arg0": [
	 *           "operator" => "stringEquals",
	 *           "arg0" => "type",
	 *           "arg1" => "bond"
	 *       ],
	 *       "arg1": [
	 *           "operator" => "stringEquals",
	 *           "arg0" => "devicename",
	 *           "arg1" => "bond0"
	 *       ]
	 *   ]
	 *   @endcode
	 * @param maxResult The maximum number of objects that are returned.
	 *   Defaults to NULL.
	 * @return An array containing the requested configuration objects. If
	 *   \em maxResult is set to 1, then the first found object is returned.
	 *   In this case the method does not return an array of configuration
	 *   objects.
	 * @throw \InvalidArgumentException
	 * @throw \OMV\Config\DatabaseException
	 */
	public function getByFilter($id, $filter, $maxResult = NULL) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Is the configuration object iterable?
		if (FALSE === $model->isIterable()) {
			throw new \InvalidArgumentException(sprintf(
			  "The configuration object '%s' is not iterable.",
			  $model->getId()));
		}
		// Build the query predicate.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildQueryByFilter($filter);
		// Redirect the query to the database backend.
		$data = $this->getBackend()->getList($xpath);
		// Create the configuration objects.
		$result = [];
		foreach ($data as $datak => $datav) {
			if (!is_null($maxResult) && ($datak >= $maxResult))
				continue;
			$object = new ConfigObject($id);
			$object->setAssoc($datav, FALSE);
			$result[] = $object;
		}
		if (1 == $maxResult) {
			if (empty($result)) {
				throw new \InvalidArgumentException(sprintf(
				  "The XPath query '%s' does not return the requested ".
				  "number of '%d' object(s).", $xpath, $maxResult));
			}
			$result = $result[0];
		}
		return $result;
	}

	public function getByFilterAssoc($id, $filter, $maxResult = NULL) {
		$objects = $this->getByFilter($id, $filter, $maxResult);
		if (1 == $maxResult) {
			$result = $objects->getAssoc();
		} else {
			$result = [];
			foreach ($objects as $objectk => $objectv)
				$result[] = $objectv->getAssoc();
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
		$queryBuilder = new DatabaseBackendQueryBuilder($object->getId());
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
		$queryBuilder = new DatabaseBackendQueryBuilder($object->getId());
		$xpath = $queryBuilder->buildDeleteQuery($object);
		if (FALSE === $this->getBackend()->delete($xpath)) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
			  $xpath);
		}
	}

	/**
	 * Delete a configuration object with the specified constraints.
	 * @param id The data model identifier.
	 * @param filter A filter specifying constraints on the objects
	 *   to retrieve.
	 * @code
	 * Example 1:
	 * [
	 *     "operator" => "not",
	 *     "arg0" => [
	 *         "operator" => "or",
	 *         "arg0" => [
	 *             "operator" => "contains",
	 *             "arg0" => "opts",
	 *             "arg1" => "bind"
	 *         ],
	 *         "arg1" => [
	 *             "operator" => "contains",
	 *             "arg0" => "opts",
	 *             "arg1" => "loop"
	 *         ]
	 *     ]
	 * ]
	 *
	 * Example 2:
	 * [
	 *     "operator" => "or",
	 *     "arg0" => [
	 *         "operator" => "stringEquals",
	 *         "arg0" => "devicefile",
	 *         "arg1" => "/dev/sda"
	 *     ],
	 *     "arg1" => [
	 *         "operator" => "stringEquals",
	 *         "arg0" => "devicefile",
	 *         "arg1" => "/dev/disk/by-id/ata-ST1000DM003-1CH132_S2DF80PC"
	 *     ]
	 * ]
	 * @endcode
	 * @return None.
	 * @throw \OMV\Config\DatabaseException
	 */
	public function deleteByFilter($id, $filter) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildQueryByFilter($filter);
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
			  "The configuration object '%s' can not be referenced.",
			  $object->getId());
		}
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($object->getId());
		$xpath = $queryBuilder->buildIsReferencedQuery($object);
		return $this->getBackend()->exists($xpath);
	}

	/**
	 * Assert that the specified configuration object is referenced.
	 * @param object The configuration object to use.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertIsReferenced(ConfigObject $object) {
		if (FALSE === $this->isReferenced($object)) {
			throw new \OMV\AssertException(
			  "The configuration object '%s' is not referenced.",
			  $object->getId());
		}
	}

	/**
	 * Assert that the specified configuration object is not referenced.
	 * @param object The configuration object to use.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertIsNotReferenced(ConfigObject $object) {
		if (TRUE === $this->isReferenced($object)) {
			throw new \OMV\AssertException(
			  "The configuration object '%s' is referenced.",
			  $object->getId());
		}
	}

	/**
	 * Check if on or more configuration object of the specified data model
	 * exists. Note, this method is only useful for iterable configuration
	 * objects.
	 * @param id The data model identifier.
	 * @param filter A filter specifying constraints on the objects
	 *   to retrieve. Defaults to NULL.
	 *   @code
	 *   Example:
	 *   [
	 *       "operator": "not",
	 *       "arg0": [
	 *           "operator" => "stringEquals",
	 *           "arg0" => "type",
	 *           "arg1" => "vlan"
	 *       ]
	 *   ]
	 *   @endcode
	 * @return TRUE if at least one configuration object exists,
	 *   otherwise FALSE.
	 */
	public function exists($id, $filter = NULL) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		if (FALSE === is_null($filter)) {
			// If a filter is set, then this means we are processing
			// iterable configuration objects. Make sure the specified
			// configuration object is iterable.
			if (FALSE === $model->isIterable()) {
				throw new \InvalidArgumentException(sprintf(
				  "The configuration object '%s' is not iterable.",
				  $model->getId()));
			}
		}
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildExistsQuery($filter);
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
		return $this->isUniqueByFilter($object, [
			"operator" => "stringEquals",
			"arg0" => $property,
			"arg1" => $object->get($property)
		]);
	}

	/**
	 * Check if a configuration object with the specified constraints
	 * is unique.
	 * @param object The configuration object to use.
	 * @param filter A filter specifying constraints on the objects
	 *   to retrieve. Defaults to NULL.
	 *   @code
	 *   Example:
	 *   [
	 *       "operator" => "stringEquals",
	 *       "arg0" => "sharename",
	 *       "arg1" => "Movies"
	 *   ]
	 *   @endcode
	 * @return TRUE if no configuration object with the same property
	 *   value exists, otherwise FALSE.
	 */
	public function isUniqueByFilter(ConfigObject $object, $filter) {
		$objects = $this->getByFilter($object->getId(), $filter);
		$numObjects = count($objects);
		if ($object->isNew())
			return (0 == $numObjects);
		return (1 == $numObjects);
	}

	/**
	 * Assert that a configuration object with the value of the specified
	 * property is unique.
	 * @param object The configuration object to use.
	 * @param property The name of the data model property.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertIsUnique(ConfigObject $object, $property) {
		if (FALSE === $this->isUnique($object, $property)) {
			throw new \OMV\AssertException("The configuration object '%s' is ".
			  "not unique. An object with the property '%s' and value '%s' ".
			  "already exists.", $object->getId(), $property,
			  $object->get($property));
		}
	}

	/**
	 * Assert that a configuration object with the specified constraints
	 * is unique.
	 * @param object The configuration object to use.
	 * @param filter A filter specifying constraints on the objects
	 *   to check.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertIsUniqueByFilter(ConfigObject $object, $filter) {
		if (FALSE === $this->isUniqueByFilter($object, $filter)) {
			throw new \OMV\AssertException("The configuration object '%s' ".
			  "with the filter '%s' is not unique.", $object->getId(),
			  json_encode($filter));
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
