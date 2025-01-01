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
namespace OMV\Rpc;

require_once("openmediavault/functions.inc");

/**
 * Helper class to handle RPC services.
 * @ingroup api
 */
class ServiceManager {
	private $map = [];

	/**
	 * Constructor
	 */
	public function __construct() {
		// Get a list of all classes that are implementing a RPC service.
		$classes = get_declared_subclasses("\OMV\Rpc\ServiceAbstract");
		foreach ($classes as $classk => $classv) {
			$this->registerService(new $classv());
		}
	}

	/**
	 * Returns a RPC manager singleton.
	 * @return The RPC manager object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new ServiceManager();
		return $instance;
	}

	/**
	 * Register a RPC service.
	 * @param service The RPC service class to be registered.
	 * @return Returns TRUE on success or FALSE on failure.
	 */
	private function registerService(ServiceAbstract $service) {
		if (!isset($service))
			return FALSE;
		$name = $service->getName();
		// Check if RPC service already exists.
		if (FALSE !== $this->getService($name)) {
			throw new \OMV\Exception(
			  "The RPC service already exists (name=%s, class=%s)",
			  $name, get_class($service));
		}
		$this->map[mb_strtolower($name)] = $service;
		ksort($this->map);
		return TRUE;
	}

	/**
	 * Get a RPC service.
	 * @param name The name of the RPC service to get.
	 * @return The RPC service object or FALSE on failure.
	 */
	final public function getService($name) {
		$name = mb_strtolower($name);
		if (!array_key_exists($name, $this->map))
			return FALSE;
		return $this->map[$name];
	}

	/**
	 * Get all registered RPC services.
	 * @return An array containing all registered RPC services.
	 */
	final public function getServices() {
		return $this->map;
	}

	/**
	 * Initialize the services.
	 */
	final public function initializeServices() {
		foreach ($this->map as $servicek => $servicev) {
			$servicev->initialize();
		}
	}

	/**
	 * Dump all registered RPC services.
	 */
	final public function dump() {
		print("Registered RPC services:\n");
		foreach ($this->map as $servicek => $servicev)
			printf("  %s\n", $servicek);
	}
}
