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
namespace OMV\Engine\Notify;

require_once("openmediavault/functions.inc");

/**
 * \OMV\Engine\Notify\Dispatcher is an configuration notification technology. It provides
 * the possibility to simply monitor various events, e.g. on the configuration.
 * @ingroup api
 */
class Dispatcher {
	use \OMV\DebugTrait;

	private static $instances = [];
	private $name = null;
	private $listeners = null;

	/**
	 * Constructor
	 * @param name The name of the dispatcher.
	 */
	function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Get the name of the dispatcher.
	 * The name is the unique identifier of a dispatcher.
	 * @return string The name of the dispatcher.
	 */
	function getName() {
		return $this->name;
	}

	/**
	 * Returns all listeners.
	 * @return array An array of listeners.
	 */
	public function getListeners() {
		if (!isset($this->listeners))
			return [];
		return $this->listeners;
	}

	/**
	 * Returns a dispatcher singleton.
	 * @param name The name of the notification dispatcher.
	 * @return object The dispatcher object.
	 */
	public static function &getInstance($name = "default") {
		if (!isset(self::$instances[$name]))
			self::$instances[$name] = new Dispatcher($name);
		return self::$instances[$name];
	}

	/**
	 * Add a listener to a given event message.
	 * @param arg1 The event message type, e.g. OMV_NOTIFY_CREATE,
	 *   OMV_NOTIFY_MODIFY, OMV_NOTIFY_PREDELETE, OMV_NOTIFY_DELETE or
	 *   OMV_NOTIFY_MODIFY|OMV_NOTIFY_DELETE.
	 * @param arg2 The notification identifier that should be monitored,
	 *   e.g. 'org.openmediavault.abc.xyz'.
	 * @param arg3 The callable to be called. A method of an instantiated
	 *   object is passed as an array containing an object at index 0 and
	 *   the method name at index 1. Accessing protected and private methods
	 *   from within a class is allowed.
	 * @param arg4 The sequence number of the called function. Default
	 *   is 20. If you want a function to be execute as the first one
	 *   then set it to 1.
	 */
	function addListener($arg1, $arg2 = null, $arg3 = null, $arg4 = 20) {
		// Bind listeners if method is called with an class object.
		if (is_a($arg1, "\OMV\Engine\Notify\IListener"))
			return $arg1->bindListeners($this);
		// Reject invalid method calls.
		if (!(is_integer($arg1) && is_string($arg2) && is_callable($arg3)))
			return;
		// Add the listerer.
		$this->debug(sprintf("Add listener (type=%s, message=%s, " .
			"callback=%s::%s, seqnum=%s", $arg1, $arg2, get_class($arg3[0]),
			$arg3[1], $arg4));
		$this->listeners[] = [
			"type" => $arg1,
			"id" => $arg2,
			"callback" => $arg3,
			"seqnum" => $arg4
		];
	}

	/**
	 * Notify all registered listeners.
	 * @param type The event message type that should be announced, e.g.
	 *   OMV_NOTIFY_CREATE, OMV_NOTIFY_MODIFY, OMV_NOTIFY_PREDELETE,
	 *   OMV_NOTIFY_DELETE or OMV_NOTIFY_MODIFY|OMV_NOTIFY_DELETE.
	 * @param id The notification identifier, e.g.
	 *   'org.openmediavault.abc.xyz'.
	 * @param argX The parameters to be passed.
	 */
	function notify($type, $id /* [, $arg1 [, $... ]] */) {
		// Filter list of registered listeners.
		$listeners = array_filter_ex($this->listeners, "id", $id);
		// Call registered listeners.
		if (!is_null($listeners)) {
			// Sort list of registered listeners.
			array_sort_key($listeners, "seqnum");
			foreach ($listeners as $listenerk => $listenerv) {
				if ($listenerv['type'] & $type) {
					$this->debug(sprintf("Notify listener (type=%s, " .
						"id=%s, callback=%s::%s, seqnum=%s",
						$listenerv['type'], $listenerv['id'],
						get_class($listenerv['callback'][0]),
						$listenerv['callback'][1], $listenerv['seqnum']));
					call_user_func_array($listenerv['callback'],
						func_get_args());
				}
			}
		}
	}

	/**
	 * Dump all registered listeners sorted by their notification identifier.
	 * @param id The notification identifier that should be monitored.
	 */
	function dumpListeners($id = "") {
		$listeners = $this->listeners;
		// Filter list of registered listeners.
		if (!empty($id)) {
			$listeners = array_filter_ex($listeners, "id", $id);
		}
		if (!empty($listeners)) {
			// Sort list of registered listeners.
			array_sort_key($listeners, "id");
			// Dump registered listeners.
			$this->debug($listeners);
		}
	}
}
