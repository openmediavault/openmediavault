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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceNetwork extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Network";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getGeneralSettings");
		$this->registerMethod("setGeneralSettings");
		$this->registerMethod("enumerateDevices");
		$this->registerMethod("enumerateDevicesList");
		$this->registerMethod("enumerateApplicableDevices");
		$this->registerMethod("identify");
		$this->registerMethod("getIface");
		$this->registerMethod("setIface");
		$this->registerMethod("deleteIface");
		$this->registerMethod("enumerateBondSlaves");
		$this->registerMethod("getBondIface");
		$this->registerMethod("setBondIface");
		$this->registerMethod("deleteBondIface");
		$this->registerMethod("getHostAccessControl");
		$this->registerMethod("setHostAccessControl");
		$this->registerMethod("getDNSNameServers");
		$this->registerMethod("setDNSNameServers");
	}

	/**
	 * Helper function to get details of the given network interface device.
	 * @param devicename The name of the network interface device.
	 * @return An array containing the network interface details. The following
	 *   fields are returned: devicename, gateway, gateway6, address, address6,
	 *   netmask, netmask6, ether, mtu, state and link.
	 */
	private function getIfaceInfo($devicename) {
		$obj = array();
		$obj['devicename'] = $devicename;
		$obj['gateway'] = "";
		$obj['gateway6'] = "";
		$obj['address'] = "";
		$obj['address6'] = "";
		$obj['netmask'] = "";
		$obj['netmask6'] = -1;
		$obj['ether'] = "";
		$obj['mtu'] = "";
		$obj['state'] = "";
		$obj['link'] = FALSE;
		$iface = new OMVNetworkInterface($devicename);
		if($iface->exists()) {
			$gateway = $iface->getGateway();
			$gateway6 = $iface->getGateway6();
			$address = $iface->getIP();
			$address6 = $iface->getIP6();
			$netmask = $iface->getMask();
			$netmask6 = $iface->getMask6();
			$ether = $iface->getMAC();
			$state = $iface->getState();
			$obj['gateway'] = !$gateway ? "" : $gateway;
			$obj['gateway6'] = !$gateway6 ? "" : $gateway6;
			$obj['address'] = !$address ? "" : $address;
			$obj['address6'] = !$address6 ? "" : $address6;
			$obj['netmask'] = !$netmask ? "" : $netmask;
			$obj['netmask6'] = !$netmask6 ? -1 : $netmask6;
			$obj['ether'] = !$ether ? "" : $ether;
			$obj['mtu'] = $iface->getMTU();
			$obj['state'] = !$state ? gettext("UNKNOWN") : $state;
			$obj['link'] = $iface->getLink();
		}
		return $obj;
	}

	/**
	 * Get general network settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getGeneralSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the network settings.
		$xpath = "//system/network";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
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
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setGeneralSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "hostname":{"type":"string","minLength":1},
				  "domainname":{"type":"string"}
			  }
		  }');
		// Get existing configuration object.
		$xpath = "//system/network";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the configuration object.
		$object = array_merge($object, array(
			"hostname" => $params['hostname'],
			"domainname" => $params['domainname']
		));
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Enumerate all network interface devices on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a network interface
	 *   device with the following properties: \em devicename, \em gateway,
	 *   \em gateway6, \em address, \em address6, \em netmask, \em netmask6,
	 *   \em ether, \em mtu, \em state and \em link.
	 * @throw E_MISC_FAILURE
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Enumerate all network interface devices on the system.
		if(FALSE === ($devs = OMVNetworkInterfaces::get())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get list of network interface devices"));
		}
		$result = array();
		// Collect network interface device details.
		foreach($devs as $devk => $devv) {
			$result[] = $this->getIfaceInfo($devv);
		}
		return $result;
	}

	/**
	 * Enumerate all network interface devices on the system, except loopback.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 * @throw E_MISC_DEVICE_NOT_SUPPORTED
	 */
	function enumerateDevicesList($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Enumerate all network interface devices on the system.
		$devs = $this->callMethod("enumerateDevices", NULL, $context);
		$result = array();
		foreach($devs as $devk => &$devv) {
			// Skip loopback network interface.
			if($devv['devicename'] == "lo")
				continue;
			// Set defaults.
			$devv['uuid'] = $GLOBALS['OMV_UUID_UNDEFINED'];
			$devv['method'] = "";
			$devv['method6'] = "";
			$devv['comment'] = "";
			$devv['_used'] = FALSE;
			$devv['_readOnly'] = FALSE;
			// Get network interface type.
			if(1 !== preg_match("/^([a-z]+)\d+$/i", $devv['devicename'],
			  $matches)) {
				throw new OMVException(
				  OMVErrorMsg::E_MISC_DEVICE_NOT_SUPPORTED,
				  $devv['devicename']);
			}
			// Check if there exists a network interface configuration
			// object for the given device.
			switch($matches[1]) {
			case "bond":
				$devv['type'] = "bond";
				$xpath = sprintf("//system/network/interfaces/bondiface[".
				  "devicename='%s']", $devv['devicename']);
				break;
			case "eth":
			case "wlan":
				$devv['type'] = "physical";
				$xpath = sprintf("//system/network/interfaces/iface[".
				  "devicename='%s']", $devv['devicename']);
				// Check if it is used by a bonding interface.
				if(TRUE === $xmlConfig->exists("//bondiface[" .
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
			if(!is_null($cfgObject)) {
				$devv['uuid'] = $cfgObject['uuid'];
				$devv['method'] = $cfgObject['method'];
				$devv['method6'] = $cfgObject['method6'];
				$devv['comment'] = $cfgObject['comment'];
			}
			// Do some more checks.
			switch($matches[1]) {
			case "bond":
				// Is bonded interface somewhere referenced?
				if(TRUE === $xmlConfig->exists("//bondifaceref[" .
				  "contains(.,'{$devv['uuid']}')]")) {
					$devv['_used'] = TRUE;
				}
				break;
			case "eth":
			default:
				// Is network interface device somewhere referenced?
				if(TRUE === $xmlConfig->exists("//ifaceref[" .
				  "contains(.,'{$devv['uuid']}')]")) {
					$devv['_used'] = TRUE;
				}
				// Check if it is used by a bonding interface.
				if(TRUE === $xmlConfig->exists("//bondiface[" .
				  "contains(slaves,'{$devv['devicename']}')]")) {
					$devv['_used'] = TRUE;
					$devv['_readOnly'] = TRUE;
					$devv['method'] = "";
					$devv['method6'] = "";
				}
				break;
			}
			// Append network interface device to result list.
			$result[] = $devv;
		}
		// Filter the result.
		return $this->applyFilter($result, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get all applicable network interfaces devices.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of applicable network interfaces devices.
	 */
	function enumerateApplicableDevices($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$result = array();
		// Get all configured physical network interface devices.
		$xpath = "//system/network/interfaces/iface/uuid";
		$uuids = $xmlConfig->getList($xpath);
		foreach($uuids as $uuid) {
			$object = $this->callMethod("getIface", array("uuid" => $uuid),
			  $context);
			// Check if it is used by a bonding interface.
			if(FALSE === $xmlConfig->exists("//bondiface[" .
			  "contains(slaves,'{$object['devicename']}')]")) {
				$object['type'] = "physical";
				$result[] = $object;
			}
		}
		// Get all configured bonded network interface devices.
		$xpath = "//system/network/interfaces/bondiface/uuid";
		$uuids = $xmlConfig->getList($xpath);
		foreach($uuids as $uuid) {
			$object = $this->callMethod("getBondIface",
			  array("uuid" => $uuid), $context);
			$object['type'] = "bond";
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Initiate adapter-specific action intended to enable an operator to
	 * easily identify the adapter by sight. Note, the communication to the
	 * system is not possible during the given time.
	 * @param params An array containing the following fields:
	 *   \em devicename The name of the network interface device, e.g. eth1.
	 *   \em seconds Length of time in seconds to blink one or more LEDs
	 *   on the specific network interface device.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_EXEC_FAILED
	 */
	function identify($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicename":{"type":"string"},
				  "seconds":{"type":"integer","minimum":1,"maximum":30}
			  }
		  }');
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Initiate adapter-specific action intended to enable an operator
			// to easily identify the adapter by sight.
			$cmd = sprintf("export LANG=C; ethtool --identify %s %d 2>&1",
			  $params['devicename'], $params['seconds']);
			if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Get a physical network interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getIface($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//system/network/interfaces/iface[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['netmask6'] = intval($object['netmask6']);
		$object['wol'] = boolval($object['wol']);
		if(!empty($object['mtu']))
			$object['mtu'] = intval($object['mtu']);
		return $object;
	}

	/**
	 * Set (add/update) a physical network interface configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setIface($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "devicename":{"type":"string"},
				  "method":{"type":"string","enum":["dhcp","static",'.
					'"manual"]},
				  "address":{"type":"string"},
				  "netmask":{"type":"string"},
				  "gateway":{"type":"string"},
				  "method6":{"type":"string","enum":["auto","static",'.
					'"manual"]},
				  "address6":{"type":"string"},
				  "netmask6":{"type":"integer","minimum":0,"maximum":128},
				  "gateway6":{"type":"string"},
				  "mtu":{"type":["integer","string"]},
				  "wol":{"type":"boolean"},
				  "options":{"type":"string"},
				  "comment":{"type":"string"}
			  }
		  }');
		// Prepare configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"devicename" => $params['devicename'],
			"method" => $params['method'],
			"address" => $params['address'],
			"netmask" => $params['netmask'],
			"gateway" => $params['gateway'],
			"method6" => $params['method6'],
			"address6" => $params['address6'],
			"netmask6" => $params['netmask6'],
			"gateway6" => $params['gateway6'],
			"mtu" => $params['mtu'],
			"wol" => array_boolval($params, 'wol'),
			"options" => $params['options'],
			"comment" => $params['comment']
		);
		// Set configuration data
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness
			$xpath = sprintf("//system/network/interfaces/iface[".
			  "devicename='%s']", $params['devicename']);
			if(TRUE === $xmlConfig->exists($xpath)) {
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
			  $params['uuid']);
			// Update existing configuration object
			$success = $xmlConfig->replace($xpath, $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.interfaces.iface", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a physical network interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	function deleteIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Delete the configuration object.
		$xpath = sprintf("//system/network/interfaces/iface[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.network.interfaces.iface");
	}

	/**
	 * Enumerate all interfaces from a bonded interface object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the bonded interface configuration object.
	 *   \em unused If TRUE then the physical network interfaces that are
	 *   not configured nor used will be appended to the result list.
	 *   Defaults to TRUE.
	 * @param context The context of the caller.
	 * @return If \em uuid is set to OMV_UUID_UNDEFINED a list of available
	 *   physical network interfaces (iface class objects) is returned,
	 *   otherwise the bonded interface slaves plus the unreferenced physical
	 *   network interfaces are returned.
	 * @throw E_MISC_DEVICE_NOT_SUPPORTED
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function enumerateBondSlaves($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "unused":{"type":"boolean"}
			  }
		  }');
		// Get list of network interfaces.
		$result = array();
		if(array_key_exists("unused", $params) && $params['unused'] === TRUE) {
			// Enumerate all network interface devices on the system.
			$devs = $this->callMethod("enumerateDevices", NULL, $context);
			// Get a list of configured physical network interfaces that are
			// not used by any other config object.
			foreach($devs as $devk => $devv) {
				// Skip loopback network interface.
				if($devv['devicename'] == "lo")
					continue;
				// Get network interface type.
				if(1 !== preg_match("/^([a-z]+)\d+$/i", $devv['devicename'],
				  $matches)) {
					throw new OMVException(
					  OMVErrorMsg::E_MISC_DEVICE_NOT_SUPPORTED,
					  $devv['devicename']);
				}
				// Is network interface device referenced somewhere?
				switch($matches[1]) {
				case "bond":
					// Skip bond interfaces.
					break;
				default:
					// Does a configuration object exists for the given
					// network interface device?
					$xpath = sprintf("//system/network/interfaces/iface[".
					  "devicename='%s']", $devv['devicename']);
					$object = $xmlConfig->get($xpath);
					if(is_null($object)) {
						// Is it referenced somewhere else?
						if(FALSE === $xmlConfig->exists("//bondiface[".
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
		if($params['uuid'] !== $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Get network interfaces already assigned to the bonding
			// interface device.
			$xpath = sprintf("//system/network/interfaces/bondiface[uuid='%s']",
			  $params['uuid']);
			$object = $xmlConfig->get($xpath);
			if(is_null($object)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			foreach(explode(",", $object['slaves']) as $slavek => $slavev) {
				$result[] = $this->getIfaceInfo($slavev);
			}
		}
		return $result;
	}

	/**
	 * Get a bonded interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getBondIface($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//system/network/interfaces/bondiface[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['netmask6'] = intval($object['netmask6']);
		$object['wol'] = boolval($object['wol']);
		if(!empty($object['mtu']))
			$object['mtu'] = intval($object['mtu']);
		$object['bondmode'] = intval($object['bondmode']);
		$object['bondmiimon'] = intval($object['bondmiimon']);
		$object['bonddowndelay'] = intval($object['bonddowndelay']);
		$object['bondupdelay'] = intval($object['bondupdelay']);
		return $object;
	}

	/**
	 * Set (add/update) a bonded interface config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setBondIface($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "devicename":{"type":"string"},
				  "method":{"type":"string","enum":["dhcp","static",'.
					'"manual"]},
				  "address":{"type":"string"},
				  "netmask":{"type":"string"},
				  "gateway":{"type":"string"},
				  "method6":{"type":"string","enum":["auto","static",'.
					'"manual"]},
				  "address6":{"type":"string"},
				  "netmask6":{"type":"integer","minimum":0,"maximum":128},
				  "gateway6":{"type":"string"},
				  "mtu":{"type":["integer","string"]},
				  "wol":{"type":"boolean"},
				  "options":{"type":"string"},
				  "comment":{"type":"string"},
				  "slaves":{"type":"string","format":"regex","pattern":'.
					'"\/^((eth|wlan)\\\d+,)*(eth|wlan)\\\d+$\/i"},
				  "bondprimary":{"type":"string"},
				  "bondmode":{"type":"integer","enum":[0,1,2,3,4,5,6]},
				  "bondmiimon":{"type":"integer","minimum":0},
				  "bonddowndelay":{"type":"integer","minimum":0},
				  "bondupdelay":{"type":"integer","minimum":0}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"method" => $params['method'],
			"address" => $params['address'],
			"netmask" => $params['netmask'],
			"gateway" => $params['gateway'],
			"method6" => $params['method6'],
			"address6" => $params['address6'],
			"netmask6" => $params['netmask6'],
			"gateway6" => $params['gateway6'],
			"mtu" => $params['mtu'],
			"wol" => array_boolval($params, 'wol'),
			"options" => $params['options'],
			"comment" => $params['comment'],
			"bondprimary" => $params['bondprimary'],
			"bondmode" => $params['bondmode'],
			"bondmiimon" => $params['bondmiimon'],
			"bonddowndelay" => $params['bonddowndelay'],
			"bondupdelay" => $params['bondupdelay'],
			"slaves" => $params['slaves']
		);
		// Set the configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$object['devicename'] = OMVSystem::getNextDevice("iface", "bond");
			// Append object to configuration
			$success = $xmlConfig->set("//system/network/interfaces",
			  array("bondiface" => $object));
		} else {
			$object['devicename'] = $params['devicename'];
			// Get existing configuration object.
			$xpath = sprintf("//system/network/interfaces/bondiface[uuid='%s']",
			  $params['uuid']);
			// Update existing configuration object.
			$success = $xmlConfig->replace($xpath, $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.interfaces.bondiface", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a bonded interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	function deleteBondIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Delete the configuration object.
		$xpath = sprintf("//system/network/interfaces/bondiface[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.network.interfaces.bondiface");
	}

	/**
	 * Get the host access control configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getHostAccessControl($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//system/network/hostac";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return array(
			"hostacallow" => $object['allow'],
			"hostacdeny" => $object['deny'],
		);
	}

	/**
	 * Set the host access control configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setHostAccessControl($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "hostacallow":{"type":"string"},
				  "hostacdeny":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"allow" => $params['hostacallow'],
			"deny" => $params['hostacdeny']
		);
		$success = $xmlConfig->replace("//system/network/hostac", $object);
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostac", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get the DNS name server configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing the configured DNS name servers.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getDNSNameServers($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//system/network";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Explode the DNS nameservers string.
		return explode(" ", $object['dnsnameservers']);
	}

	/**
	 * Set the DNS name server configuration.
	 * @param params An array containing the following fields:
	 *   \em dnsnameservers An array containing the DNS name servers.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setDNSNameServers($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "dnsnameservers":{
					  "type":"array",
					  "items":{
						  '.$GLOBALS['OMV_JSONSCHEMA_IPV4'].'
					  }
				  }
			  }
		  }');
		// Prepare the configuration object.
		$object = implode(" ", $params['dnsnameservers']);
		$success = $xmlConfig->replace("//system/network/dnsnameservers",
		  $object);
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.dnsnameservers", $object);
		// Return the configuration object.
		return $object;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceNetwork());
?>
