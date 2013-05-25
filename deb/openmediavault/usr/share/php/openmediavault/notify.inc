<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
require_once("openmediavault/globals.inc");
require_once("openmediavault/object.inc");

/**
 * OMVNotifyDispatcher is an configuration notification technology. It provides
 * the possibility to simply monitor various events, e.g. on the configuration.
 * @ingroup api
 */
class OMVNotifyDispatcher extends OMVObject {
	private static $instances = array();
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
	 * @return The name of the dispatcher.
	 */
	function getName() {
		return $this->name;
	}

	/**
	 * Returns all listeners.
	 * @return An array of listeners.
	 */
	public function getListeners() {
		if(!isset($this->listeners)) {
			return array();
		}
		return $this->listeners;
	}

	/**
	 * Returns a dispatcher singleton.
	 * @param name The name of the notification dispatcher.
	 * @return The dispatcher object.
	 */
	public static function &getInstance($name = "default") {
		if(!isset(self::$instances[$name])) {
			self::$instances[$name] = new OMVNotifyDispatcher($name);
		}
		return self::$instances[$name];
	}

	/**
	 * Add a listener to a given event message.
	 * @param arg1 The event type, e.g. OMV_NOTIFY_CREATE. OMV_NOTIFY_MODIFY,
	 * OMV_NOTIFY_PREDELETE, OMV_NOTIFY_DELETE or
	 * OMV_NOTIFY_MODIFY|OMV_NOTIFY_DELETE.
	 * @param arg2 The event message that should be monitored.
	 * @param arg3 The callback function.
	 * @param arg4 The sequence number of the called function. Default is 20.
	 * If you want a function to be execute as the first one then set it to 1.
	 */
	function addListener($arg1, $arg2 = null, $arg3 = null, $arg4 = 20) {
		if(is_a($arg1, "OMVINotifyListener")) {
			return $arg1->bindListeners($this);
		}
		// Add listerer
//		$this->debug(sprintf("Add listener (type=%s, message=%s, " .
//		  "callback=%s::%s, seqnum=%s", $arg1, $arg2, get_class($arg3[0]),
//		  $arg3[1], $arg4));
		$this->listeners[] = array(
			"type" => $arg1,
			"message" => $arg2,
			"callback" => $arg3,
			"seqnum" => $arg4
		);
	}

	/**
	 * Notify all registered listeners.
	 * @param type The event type that should be announced.
	 * @param message The event message.
	 * @param argX The parameters to be passed.
	 */
	function notify($type, $message /* [, $arg1 [, $... ]] */) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Filter list of registered listeners
		$listeners = array_filter_ex($this->listeners, "message", $message);
		// Sort list of registered listeners
		array_sort_key($listeners, "seqnum");
		// Call registered listeners
		if(!empty($listeners)) {
			foreach($listeners as $listenerk => $listenerv) {
				if($listenerv['type'] & $type) {
//					$this->debug(sprintf("Notify listener (type=%s, " .
//					  "message=%s, callback=%s::%s, seqnum=%s",
//					  $listenerv['type'], $listenerv['message'],
//					  get_class($listenerv['callback'][0]),
//					  $listenerv['callback'][1], $listenerv['seqnum']));
					$args = array_slice(func_get_args(), 2);
					call_user_func_array($listenerv['callback'], $args);
				}
			}
		}
	}

	/**
	 * Dump all registered listeners sorted by their event message.
	 * @param message The event message that should be monitored.
	 */
	function dumpListeners($message = "") {
		$listeners = $this->listeners;
		// Filter list of registered listeners
		if(!empty($message)) {
			$listeners = array_filter_ex($listeners, "message", $message);
		}
		// Sort list of registered listeners
		array_sort_key($listeners, "message");
		// Dump registered listeners
		$this->debug($listeners);
	}
}

/**
 * Interface that must be implemented to be able to listen for event
 * notifications.
 * @ingroup api
 */
interface OMVINotifyListener {
	/**
	 * Bind listeners.
	 * @param dispatcher The dispatcher object.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher);
}
?>
