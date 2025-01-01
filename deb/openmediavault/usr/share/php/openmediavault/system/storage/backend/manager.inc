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
namespace OMV\System\Storage\Backend;

/**
 * Helper class to get all existing devices of the given storage type.
 * @ingroup api
 */
class Manager implements \IteratorAggregate {
	private $backends = [];

	/**
	 * Returns a manager singleton.
	 * @return The manager object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new Manager();
		return $instance;
	}

	public function getIterator() {
		return new \ArrayIterator($this->backends);
	}

	/**
	 * Register a storage device backend. If the registered backend class is
	 * derived from another backend, then it will be inserted before this.
	 * @param backend The storage device backend object to register.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final public function registerBackend(BackendAbstract $backend) {
		if (!isset($backend))
			return FALSE;
		if (FALSE === ($parent = get_parent_class($backend))) {
			$this->backends[] = $backend;
		} else {
			// Try to find the derived class and the position where to
			// insert the storage device backend class to register.
			$offset = FALSE;
			foreach ($this->backends as $backendk => $backendv) {
				if (("\OMV\System\Storage\Backend\BackendAbstract" == $parent) ||
				  (FALSE === ($backendv instanceof $parent)))
					continue;
				$offset = $backendk;
				break;
			}
			if (FALSE === $offset) {
				$this->backends[] = $backend;
			} else {
				array_splice($this->backends, $offset, 0, array($backend));
			}
		}
		return TRUE;
	}

	/**
	 * Get the backend of the given device.
	 * @param deviceFile Specifies the device file.
	 * @return The backend that implements the given device file, otherwise
	 *   NULL.
	 */
	final public function getBackend($deviceFile) {
		$result = NULL;
		foreach ($this->backends as $backendk => $backendv) {
			if (FALSE === $backendv->isTypeOf($deviceFile))
				continue;
			$result = $backendv;
			break;
		}
		return $result;
	}

	/**
	 * Assert that a backend for the given device exists.
	 * @param deviceFile Specifies the device file.
	 * @throw \OMV\AssertException
	 */
	final public function assertBackendExists($deviceFile) {
		$result = $this->getBackend($deviceFile);
		if (is_null($result)) {
			throw new \OMV\AssertException(
			  "No storage device backend exists for the device '%s'.",
			  $deviceFile);
		}
	}
}
