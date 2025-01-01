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
namespace OMV\Config;

/**
 * @ingroup api
 */
class Database {
	private $backend = NULL;

	public function __construct() {
		$this->backend = new DatabaseBackend(\OMV\Environment::get(
		  "OMV_CONFIG_FILE"));
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

	/**
	 * Get the object that implements the database backend.
	 * @return The singleton object.
	 */
	public function getBackend() {
		return $this->backend;
	}

	/**
	 * Lock the database.
	 * @param boolean $auto Automatically acquire the lock and release
	 *   it on destruction. Defaults to TRUE.
	 * @return The mutex object.
	 */
	public function lock($auto = TRUE): \OMV\Mutex {
		return $this->getBackend()->lock();
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
				// Do not validate the properties because they surely
				// do not match the specified types of the model when
				// they are loaded as string data from XML.
				$object->setAssoc($datav, FALSE);
				// Now we can validate the object after the properties
				// have been auto-converted to their correct types.
				$object->validate();
				$result[] = $object;
			}
		} else {
			$result = new ConfigObject($id);
			// Do not validate the properties because they surely
			// do not match the specified types of the model when
			// they are loaded as string data from XML.
			$result->setAssoc($data, FALSE);
			// Now we can validate the object after the properties
			// have been auto-converted to their correct types.
			$result->validate();
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
	 * @return array An array containing the requested configuration objects.
	 *   If \em maxResult is set to 1, then the first found object is returned.
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
			// Do not validate the properties because they surely
			// do not match the specified types of the model when
			// they are loaded as string data from XML.
			$object->setAssoc($datav, FALSE);
			// Now we can validate the object after the properties
			// have been auto-converted to their correct types.
			$object->validate();
			$result[] = $object;
		}
		if (1 == $maxResult) {
			if (empty($result)) {
				throw new DatabaseException(sprintf(
				  "The XPath query '%s' does not return the requested ".
				  "number of %d object(s).", $xpath, $maxResult));
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
	 * @param quiet If set to TRUE, then no notification message is send.
	 *   Defaults to FALSE.
	 * @return void
	 * @throw \OMV\Config\DatabaseException
	 */
	public function set(ConfigObject &$object, $quiet = FALSE) {
		$oldObjectData = NULL;
		// Get the notification settings.
		$notifyType = OMV_NOTIFY_MODIFY;
		$notifyId = $object->getModel()->getNotificationId();
		// Build the XPath query string.
		$queryBuilder = new DatabaseBackendQueryBuilder($object->getModelId());
		$xpath = $queryBuilder->buildSetQuery($object);
		if ((TRUE === $object->isIterable()) && (TRUE === $object->isNew())) {
			$notifyType = OMV_NOTIFY_CREATE;
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
			// Get the data of the configuration object.
			$objectData = $object->getAssoc();
			// Finally put the configuration object.
			$result = $this->getBackend()->set($xpath, [
				$nodeName => $objectData
			]);
		} else {
			// Get the old configuration object.
			$uuid = NULL;
			if ($object->isIdentifiable())
				$uuid = $object->getIdentifier();
			$oldObjectData = $this->getAssoc($object->getModelId(), $uuid);
			// Get the data of the configuration object.
			$objectData = $object->getAssoc();
			// Finally put the configuration object.
			$result = $this->getBackend()->replace($xpath, $objectData);
		}
		if (FALSE === $result) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
			  $xpath);
		}
		// Submit the notification message.
		if (FALSE === $quiet) {
			$notifyDispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$notifyDispatcher->notify($notifyType, $notifyId, $objectData,
			  $oldObjectData);
		}
	}

	/**
	 * Delete the specified configuration object.
	 * @param object The configuration object to use.
	 * @param quiet If set to TRUE, then no notification message is send.
	 *   Defaults to FALSE.
	 * @return void
	 * @throw \OMV\Config\DatabaseException
	 */
	public function delete(ConfigObject $object, $quiet = FALSE) {
		// Get the notification identifier and the data to be submitted.
		$notifyId = $object->getModel()->getNotificationId();
		$objectData = $object->getAssoc();
		// Build the XPath query string.
		$queryBuilder = new DatabaseBackendQueryBuilder($object->getModelId());
		$xpath = $queryBuilder->buildDeleteQuery($object);
		// Submit the notification message.
		if (FALSE === $quiet) {
			$notifyDispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$notifyDispatcher->notify(OMV_NOTIFY_PREDELETE, $notifyId,
			  $objectData);
		}
		// Delete the object.
		if (FALSE === $this->getBackend()->delete($xpath)) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
			  $xpath);
		}
		// Submit the notification message.
		if (FALSE === $quiet) {
			$notifyDispatcher->notify(OMV_NOTIFY_DELETE, $notifyId,
			  $objectData);
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
	 * @param quiet If set to TRUE, then no notification message is send.
	 *   Defaults to FALSE.
	 * @return void
	 * @throw \OMV\Config\DatabaseException
	 */
	public function deleteByFilter($id, $filter, $quiet = FALSE) {
		// Get the specified data model.
		$mngr = \OMV\DataModel\Manager::getInstance();
		$model = $mngr->getModel($id);
		// Get the objects to be deleted.
		$objects = [];
		if (FALSE === $quiet)
			$objects = $this->getByFilter($id, $filter);
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->buildQueryByFilter($filter);
		if (FALSE === $this->getBackend()->delete($xpath)) {
			throw new DatabaseException("Failed to execute XPath query '%s'.",
			  $xpath);
		}
		// Submit the notification message.
		if (FALSE === $quiet) {
			$notifyDispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			foreach ($objects as $objectk => $objectv) {
				$notifyDispatcher->notify(OMV_NOTIFY_DELETE,
					$model->getNotificationId(),
					$objectv->getAssoc());
			}
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
			  $object->getModelId());
		}
		// Create the query builder.
		$queryBuilder = new DatabaseBackendQueryBuilder($object->getModelId());
		$xpath = $queryBuilder->buildIsReferencedQuery($object);
		return $this->getBackend()->exists($xpath);
	}

	/**
	 * Assert that the specified configuration object is referenced.
	 * @param object The configuration object to use.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertIsReferenced(ConfigObject $object) {
		if (FALSE === $this->isReferenced($object)) {
			throw new \OMV\AssertException(
			  "The configuration object '%s' is not referenced.",
			  $object->getModelId());
		}
	}

	/**
	 * Assert that the specified configuration object is not referenced.
	 * @param object The configuration object to use.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertIsNotReferenced(ConfigObject $object) {
		if (TRUE === $this->isReferenced($object)) {
			throw new \OMV\AssertException(
			  "The configuration object '%s' is referenced.",
			  $object->getModelId());
		}
	}

	/**
	 * Check if on or more configuration object of the specified data model
	 * exists.
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
	 *   to retrieve.
	 *   @code
	 *   Example:
	 *   [
	 *       "operator" => "stringEquals",
	 *       "arg0" => "sharename",
	 *       "arg1" => "Movies"
	 *   ]
	 *   @endcode
	 * @return TRUE if no configuration object with the same property
	 *   values (specified by the filter) exists, otherwise FALSE.
	 */
	public function isUniqueByFilter(ConfigObject $object, $filter) {
		// If the object is iterable and not new, then we need to modify the
		// filter to do not find the object itself.
		if ((FALSE === $object->isNew()) && (TRUE === $object->isIterable())) {
			$idProperty = $object->getModel()->getIdProperty();
			$filter = [
				"operator" => "and",
 				"arg0" => [
 					"operator" => "stringNotEquals",
 					"arg0" => $idProperty,
 					"arg1" => $object->get($idProperty)
 				],
 				"arg1" => $filter
			];
		}
		$objects = $this->getByFilter($object->getModelId(), $filter);
		return (0 == count($objects));
	}

	/**
	 * Assert that a configuration object with the value of the specified
	 * property is unique.
	 * @param object The configuration object to use.
	 * @param property The name of the data model property.
	 * @param message The exception message. If NULL, a default message
	 *   is generated. Defaults to NULL.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertIsUnique(ConfigObject $object, $property, $message = NULL) {
		if (FALSE === $this->isUnique($object, $property)) {
			if (is_null($message)) {
				$message = sprintf("The configuration object '%s' is ".
					"not unique. An object with the property '%s' and ".
					"value '%s' already exists.",
					$object->getModelId(), $property,
					$object->get($property));
			}
			throw new \OMV\AssertException($message);
		}
	}

	/**
	 * Assert that a configuration object with the specified constraints
	 * is unique.
	 * @param object The configuration object to use.
	 * @param filter A filter specifying constraints on the objects
	 *   to check.
	 * @param message The exception message. If NULL, a default message
	 *   is generated. Defaults to NULL.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertIsUniqueByFilter(ConfigObject $object, $filter, $message = NULL) {
		if (FALSE === $this->isUniqueByFilter($object, $filter)) {
			if (is_null($message)) {
				$message = sprintf("The configuration object '%s' with the ".
					"filter '%s' is not unique.", $object->getModelId(),
			  		json_encode($filter));
			}
			throw new \OMV\AssertException($message);
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
	 * @return void
	 */
	public function revert($filename) {
		$this->getBackend()->revert($filename);
	}
}
