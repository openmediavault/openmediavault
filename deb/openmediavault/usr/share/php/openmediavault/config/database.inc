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
	 * @param xpath The XPath expression to execute.
	 * @param id The data model identifier. If \em id is not set, then
	 *   the xpath will be used as identifier instead.
	 * @return The configuration object.
	 */
	public function get($id, ...$args) {
		$queryBuilder = new DatabaseBackendQueryBuilder($id);
		$xpath = $queryBuilder->getGetQuery($args);
		if (0 < count($args))
			$data = $this->getBackend()->get($xpath, ...$args);
		else
			$data = $this->getBackend()->getList($xpath);
		if (!is_array($data)) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
		if (0 < count($args)) {
			$result = array();
			foreach ($data as $datak => $datav) {
				$object = new ConfigObject($id);
				$object->setAssoc($datav, FALSE);
				$objects[] = $object;
			}
		} else {
			$result = new ConfigObject($id);
			$result->setAssoc($data);
		}
		return $result;
	}

	/**
	 * Set the configuration object at the specified XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @param object The configuration object to use.
	 * @return None.
	 */
	public function set(ConfigObject $object) {
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->getSetQuery($object);
		if (TRUE === $object->isNew()) {
			$success = $this->getBackend()->set($xpath,
			  $queryBuilder->getValue($object));
		} else {
			$success = $this->getBackend()->replace($xpath,
			  $queryBuilder->getValue($object));
		}
		if (FALSE === $success) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
	}

	/**
	 * Delete the nodes matching the specified XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @return None.
	 */
	public function delete(ConfigObject $object) {
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->getDeleteQuery($object);
		if (FALSE === $this->getBackend()->delete($xpath)) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
	}

	/**
	 * Check if the configuration at the given XPath expression exists.
	 * @param xpath The XPath expression to execute.
	 * @return TRUE if the configuration exists, otherwise FALSE.
	 */
	public function exists($xpath) {
		return $this->getBackend()->exists($xpath);
	}

	public function isReferenced(ConfigObject $object) {
		if (FALSE === $this->getModel()->isReferenceable()) {
			throw new DatabaseException(
			  "The configuration object can not be referenced.");
		}
		$queryBuilder = new DatabaseBackendQueryBuilder(
		  $object->getModel()->getId());
		$xpath = $queryBuilder->getIsReferencedQuery($object);
		if (FALSE === $this->getBackend()->exists($xpath)) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
	}
}
