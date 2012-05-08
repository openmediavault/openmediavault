<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class NetworkRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setGeneralSettings" => array('{
				"type":"object",
				"properties":{
					"hostname":{"type":"string","minLength":1},
					"domainname":{"type":"string"}
				}
			}'),
			"enumerateDevicesList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"identify" => array('{
				"type":"object",
				"properties":{
					"devicename":{"type":"string"},
					"seconds":{"type":"integer","minimum":1,"maximum":30}
				}
			}'),
			"getIface" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setIface" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"devicename":{"type":"string"},
					"method":{"type":"string","enum":["dhcp","static",'.
					  '"manual"]},
					"address":{"type":"string"},
					"netmask":{"type":"string"},
					"gateway":{"type":"string"},
					"mtu":{"type":["integer","string"]},
					"wol":{"type":"boolean"},
					"options":{"type":"string"}
				}
			}'),
			"deleteIface" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"enumerateBondSlaves" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"unused":{"type":"boolean"}
				}
			}'),
			"getBondIface" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setBondIface" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"devicename":{"type":"string"},
					"method":{"type":"string","enum":["dhcp","static",'.
					  '"manual"]},
					"address":{"type":"string"},
					"netmask":{"type":"string"},
					"gateway":{"type":"string"},
					"mtu":{"type":["integer","string"]},
					"wol":{"type":"boolean"},
					"options":{"type":"string"},
					"slaves":{"type":"string","format":"regex","pattern":'.
					  '"\/^((eth|wlan)\\\d+,)*(eth|wlan)\\\d+$\/i"},
					"bondprimary":{"type":"string"},
					"bondmode":{"type":"integer","enum":[0,1,2,3,4,5,6]},
					"bondmiimon":{"type":"integer","minimum":0},
					"bonddowndelay":{"type":"integer","minimum":0},
					"bondupdelay":{"type":"integer","minimum":0}
				}
			}'),
			"deleteBondIface" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setHostAccessControl" => array('{
				"type":"object",
				"properties":{
					"hostacallow":{"type":"string"},
					"hostacdeny":{"type":"string"}
				}
			}'),
			"setDNSNameServers" => array('{
				"type":"object",
				"properties":{
					"primarydns":{'.$GLOBALS['OMV_JSONSCHEMA_IPV4_NONE'].'},
					"secondarydns":{'.$GLOBALS['OMV_JSONSCHEMA_IPV4_NONE'].'}
				}
			}'),
			"setDNSServiceDiscovery" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"}
				}
			}')
		);
	}

	/**
	 * Helper function to get details of the given network interface device.
	 * @param devicename The name of the network interface device.
	 * @return An array containing the network interface details. The following
	 * fields are returned: devicename, gateway, address, address6, netmask,
	 * ether, mtu, state and link.
	 */
	private function getIfaceInfo($devicename) {
		$obj = array();
		$obj['devicename'] = $devicename;
		$obj['gateway'] = "";
		$obj['address'] = "";
		$obj['address6'] = "";
		$obj['netmask'] = "";
		$obj['ether'] = "";
		$obj['mtu'] = "";
		$obj['state'] = "";
		$obj['link'] = FALSE;
		$iface = new OMVNetworkInterface($devicename);
		if ($iface->exists()) {
			$gateway = $iface->getGateway();
			$address = $iface->getIPv4();
			$address6 = $iface->getIPv6();
			$netmask = $iface->getMask();
			$ether = $iface->getMAC();
			$state = $iface->getState();
			$obj['gateway'] = !$gateway ? "" : $gateway;
			$obj['address'] = !$address ? "" : $address;
			$obj['address6'] = !$address6 ? "" : $address6;
			$obj['netmask'] = !$netmask ? "" : $netmask;
			$obj['ether'] = !$ether ? "" : $ether;
			$obj['mtu'] = $iface->getMTU();
			$obj['state'] = !$state ? "UNKNOWN" : $state;
			$obj['link'] = $iface->getLink();
		}
		return $obj;
	}

	/**
	 * Get general network settings.
	 */
	function getGeneralSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get the network settings
		$xpath = "//system/network";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return array(
			"hostname" => $object['hostname'],
			"domainname" => $object['domainname']
		);
	}

	/**
	 * Set general network settings.
	 */
	function setGeneralSettings($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get existing configuration object
		$xpath = "//system/network";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration
		$object = array_merge($object, array(
			"hostname" => $data['hostname'],
			"domainname" => $data['domainname']
		));
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname", $object);
	}

	/**
	 * Enumerate all network interface devices on the system.
	 * @return An array of objects. Each object represents a network interface
	 * device with the following properties: devicename, gateway, address,
	 * address6, netmask, ether, mtu, state and link.
	 */
	public function enumerateDevices() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Enumerate all network interface devices on the system.
		if (FALSE === ($devs = OMVNetworkInterfaces::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of network interface devices");
		}
		$result = array();
		// Collect network interface device details.
		foreach ($devs as $devk => $devv) {
			$result[] = $this->getIfaceInfo($devv);
		}
		return $result;
	}

	/**
	 * Enumerate all network interface devices on the system, except loopback.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function enumerateDevicesList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Enumerate all network interface devices on the system.
		$devs = $this->enumerateDevices();
		$result = array();
		foreach ($devs as $devk => &$devv) {
			// Skip loopback network interface.
			if ($devv['devicename'] === "lo")
				continue;
			// Set defaults.
			$devv['uuid'] = $GLOBALS['OMV_UUID_UNDEFINED'];
			$devv['method'] = "";
			$devv['_used'] = FALSE;
			$devv['_readOnly'] = FALSE;
			// Get network interface type.
			if (1 !== preg_match("/^([a-z]+)\d+$/i", $devv['devicename'],
			  $matches)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
				  "Unsupported network interface detected (name=%s)",
				  $devv['devicename']));
			}
			// Check if there exists a network interface configuration
			// object for the given device.
			switch ($matches[1]) {
			case "bond":
				$devv['type'] = "bond";
				$xpath = sprintf("//system/network/interfaces/bondiface[".
				  "devicename='%s']", $devv['devicename']);
				break;
			case "eth":
				$devv['type'] = "physical";
				$xpath = sprintf("//system/network/interfaces/iface[".
				  "devicename='%s']", $devv['devicename']);
				// Check if it is used by a bonding interface.
				if (TRUE === $xmlConfig->exists("//bondiface[" .
				  "contains(slaves,'{$devv['devicename']}')]")) {
					$devv['_used'] = TRUE;
				}
				break;
			default:
				$devv['type'] = "unknown";
				$xpath = sprintf("//system/network/interfaces/iface[".
				  "devicename='%s']", $devv['devicename']);
				break;
			}
			$cfgObject = $xmlConfig->get($xpath);
			if (!is_null($cfgObject)) {
				$devv['uuid'] = $cfgObject['uuid'];
				$devv['method'] = $cfgObject['method'];
			}
			// Do some more checks.
			switch ($matches[1]) {
			case "bond":
				// Is bonded interface somewhere referenced?
				if (TRUE === $xmlConfig->exists("//bondifaceref[" .
				  "contains(.,'{$devv['uuid']}')]")) {
					$devv['_used'] = TRUE;
				}
				break;
			case "eth":
			default:
				// Is network interface device somewhere referenced?
				if (TRUE === $xmlConfig->exists("//ifaceref[" .
				  "contains(.,'{$devv['uuid']}')]")) {
					$devv['_used'] = TRUE;
				}
				// Check if it is used by a bonding interface.
				if (TRUE === $xmlConfig->exists("//bondiface[" .
				  "contains(slaves,'{$devv['devicename']}')]")) {
					$devv['_used'] = TRUE;
					$devv['_readOnly'] = TRUE;
					$devv['method'] = "";
				}
				break;
			}
			// Append network interface device to result list.
			$result[] = $devv;
		}
		// Filter result
		return $this->applyFilter($result, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get all applicable network interfaces devices.
	 */
	function enumerateApplicableDevices() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		$result = array();
		// Get all configured physical network interface devices.
		$xpath = "//system/network/interfaces/iface/uuid";
		$uuids = $xmlConfig->getList($xpath);
		foreach ($uuids as $uuid) {
			$object = OMVRpc::exec("Network", "getIface", array(
			  "uuid" => $uuid));
			// Check if it is used by a bonding interface.
			if (FALSE === $xmlConfig->exists("//bondiface[" .
			  "contains(slaves,'{$object['devicename']}')]")) {
				$object['type'] = "physical";
				$result[] = $object;
			}
		}
		// Get all configured bonded network interface devices.
		$xpath = "//system/network/interfaces/bondiface/uuid";
		$uuids = $xmlConfig->getList($xpath);
		foreach ($uuids as $uuid) {
			$object = OMVRpc::exec("Network", "getBondIface", array(
			  "uuid" => $uuid));
			$object['type'] = "bond";
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Initiate adapter-specific action intended to enable an operator to
	 * easily identify the adapter by sight. Note, the communication to the
	 * system is not possible during the given time.
	 * @param data An array containing the following fields:
	 *   \em devicename The name of the network interface device, e.g. eth1.
	 *   \em seconds Length of time in seconds to blink one or more LEDs
	 *   on the specific network interface device.
	 * @return The PID of the process started to identify the network
	 *   interface device.
	 */
	function identify($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions, but do not commit session.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Initiate adapter-specific action intended to enable an operator to
		// easily identify the adapter by sight.
		$outputFile = tempnam(sys_get_temp_dir(), "ethtool");
		$cmd = sprintf("export LANG=C; sudo ethtool --identify %s %d ".
		  ">%s 2>&1 & echo $!", $data['devicename'], $data['seconds'],
		  $outputFile);
		OMVUtil::exec($cmd, $output, $res);
		if ($res !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Remember pid and content filename in session.
		$pid = intval($output[0]);
		$_SESSION['execjob'][$pid] = array(
			"pid" => $pid,
			"outputFile" => $outputFile
		);
		return $pid;
	}

	/**
	 * Get a physical network interface configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	function getIface($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = sprintf("//system/network/interfaces/iface[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['wol'] = boolval($object['wol']);
		if (!empty($object['mtu']))
			$object['mtu'] = intval($object['mtu']);
		return $object;
	}

	/**
	 * Set (add/update) a physical network interface configuration object
	 */
	function setIface($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"devicename" => $data['devicename'],
			"method" => $data['method'],
			"address" => isset($data['address']) ? $data['address'] : "",
			"netmask" => isset($data['netmask']) ? $data['netmask'] : "",
			"gateway" => isset($data['gateway']) ? $data['gateway'] : "",
			"mtu" => $data['mtu'],
			"wol" => array_boolval($data, 'wol'),
			"options" => $data['options']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness
			$xpath = sprintf("//system/network/interfaces/iface[".
			  "devicename='%s']", $data['devicename']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  sprintf(gettext("Network interface '%s' already exists"),
				  $object['devicename']));
			}
			// Append object to configuration
			$success = $xmlConfig->set("//system/network/interfaces",
			  array("iface" => $object));
		} else {
			// Get existing configuration object
			$xpath = sprintf("//system/network/interfaces/iface[uuid='%s']",
			  $data['uuid']);
			// Update existing configuration object
			$success = $xmlConfig->replace($xpath, $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.interfaces.iface", $object);
	}

	/**
	 * Delete a physical network interface configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	function deleteIface($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Delete configuration object
		$xpath = sprintf("//system/network/interfaces/iface[uuid='%s']",
		  $data['uuid']);
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.iface", $object);
	}

	/**
	 * Enumerate all interfaces from a bonded interface object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the bonded interface configuration object.
	 *   \em unused If TRUE then the physical network interfaces that are
	 *   not configured nor used will be appended to the result list.
	 *   Defaults to TRUE.
	 * @return If \em uuid is set to OMV_UUID_UNDEFINED a list of available
	 *   physical network interfaces (iface class objects) is returned,
	 *   otherwise the bonded interface slaves plus the unreferenced physical
	 *   network interfaces are returned.
	 */
	public function enumerateBondSlaves($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get list of network interfaces.
		$result = array();
		if (array_key_exists("unused", $data) && $data['unused'] === TRUE) {
			// Enumerate all network interface devices on the system.
			$devs = $this->enumerateDevices();
			// Get a list of configured physical network interfaces that are
			// not used by any other config object.
			foreach ($devs as $devk => $devv) {
				// Skip loopback network interface.
				if ($devv['devicename'] === "lo")
					continue;
				// Get network interface type.
				if (1 !== preg_match("/^([a-z]+)\d+$/i", $devv['devicename'],
				  $matches)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
					  "Unsupported network interface detected (name=%s)",
					  $devv['devicename']));
				}
				// Is network interface device referenced somewhere?
				switch ($matches[1]) {
				case "bond":
					// Skip bond interfaces.
					break;
				default:
					// Does a configuration object exists for the given
					// network interface device?
					$xpath = sprintf("//system/network/interfaces/iface[".
					  "devicename='%s']", $devv['devicename']);
					$object = $xmlConfig->get($xpath);
					if (is_null($object)) {
						// Is it referenced somewhere else?
						if (FALSE === $xmlConfig->exists("//bondiface[".
						  "contains(slaves,'{$devv['devicename']}')]")) {
							$result[] = $devv;
						}
					} else {
						$result[] = $devv;
					}
					break;
				}
			}
		}
		if ($data['uuid'] !== $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Get network interfaces already assigned to the bonding
			// interface device.
			$xpath = sprintf("//system/network/interfaces/bondiface[uuid='%s']",
			  $data['uuid']);
			$object = $xmlConfig->get($xpath);
			if (is_null($object)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			foreach (explode(",", $object['slaves']) as $slavek => $slavev) {
				$result[] = $this->getIfaceInfo($slavev);
			}
		}
		return $result;
	}

	/**
	 * Get a bonded interface configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	function getBondIface($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = sprintf("//system/network/interfaces/bondiface[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['wol'] = boolval($object['wol']);
		if (!empty($object['mtu']))
			$object['mtu'] = intval($object['mtu']);
		$object['bondmode'] = intval($object['bondmode']);
		$object['bondmiimon'] = intval($object['bondmiimon']);
		$object['bonddowndelay'] = intval($object['bonddowndelay']);
		$object['bondupdelay'] = intval($object['bondupdelay']);
		return $object;
	}

	/**
	 * Set (add/update) a bonded interface config object
	 */
	function setBondIface($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"method" => $data['method'],
			"address" => isset($data['address']) ? $data['address'] : "",
			"netmask" => isset($data['netmask']) ? $data['netmask'] : "",
			"gateway" => isset($data['gateway']) ? $data['gateway'] : "",
			"mtu" => $data['mtu'],
			"wol" => array_boolval($data, 'wol'),
			"options" => $data['options'],
			"bondprimary" => $data['bondprimary'],
			"bondmode" => $data['bondmode'],
			"bondmiimon" => $data['bondmiimon'],
			"bonddowndelay" => $data['bonddowndelay'],
			"bondupdelay" => $data['bondupdelay'],
			"slaves" => $data['slaves']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$object['devicename'] = OMVSystem::getNextDevice("iface", "bond");
			// Append object to configuration
			$success = $xmlConfig->set("//system/network/interfaces",
			  array("bondiface" => $object));
		} else {
			$object['devicename'] = $data['devicename'];
			// Get existing configuration object
			$xpath = sprintf("//system/network/interfaces/bondiface[uuid='%s']",
			  $data['uuid']);
			// Update existing configuration object
			$success = $xmlConfig->replace($xpath, $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.interfaces.bondiface", $object);
	}

	/**
	 * Delete a bonded interface configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	function deleteBondIface($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Delete configuration object
		$xpath = sprintf("//system/network/interfaces/bondiface[uuid='%s']",
		  $data['uuid']);
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.bondiface", $object);
	}

	/**
	 * Get the host access control configuration
	 */
	function getHostAccessControl() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		$xpath = "//system/network/hostac";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return array(
			"hostacallow" => $object['allow'],
			"hostacdeny" => $object['deny'],
		);
	}

	/**
	 * Set the host access control configuration
	 */
	function setHostAccessControl($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data
		$object = array(
			"allow" => $data['hostacallow'],
			"deny" => $data['hostacdeny']
		);
		$success = $xmlConfig->replace("//system/network/hostac", $object);
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostac", $object);
	}

	/**
	 * Get the DNS name server configuration
	 */
	function getDNSNameServers() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//system/network";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Explode the DNS nameservers string. Note, only the first two
		// addresses are used.
		$object = explode(" ", $object['dnsnameservers']);
		return array(
			"primarydns" => isset($object[0]) ? $object[0] : "",
			"secondarydns" => isset($object[1]) ? $object[1] : ""
		);
	}

	/**
	 * Set the DNS name server configuration
	 */
	function setDNSNameServers($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data
		$object = trim("{$data['primarydns']} {$data['secondarydns']}");
		$success = $xmlConfig->replace("//system/network/dnsnameservers",
		  $object);
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.dnsnameservers", $object);
	}

	/**
	 * Get the DNS service discovery configuration
	 */
	function getDNSServiceDiscovery() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//system/network/dnsservicediscovery";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set the DNS service discovery configuration
	 */
	function setDNSServiceDiscovery($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Update configuration
		$xpath = "//system/network/dnsservicediscovery";
		$object = array(
			"enable" => array_boolval($data, 'enable')
		);
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.dnsservicediscovery", $object);
	}
}
?>
