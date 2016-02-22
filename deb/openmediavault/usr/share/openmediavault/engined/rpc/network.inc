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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

$GLOBALS['OMV_JSONSCHEMA_NETWORK_INTERFACE_GENERIC'] =
  '"devicename":{"type":"string"},'.
  '"method":{"type":"string","enum":["dhcp","static","manual"]},'.
  '"address":{"type":"string"},'.
  '"netmask":{"type":"string"},'.
  '"gateway":{"type":"string"},'.
  '"method6":{"type":"string","enum":["auto","static","manual","dhcp"]},'.
  '"address6":{"type":"string"},'.
  '"netmask6":{"type":"integer","minimum":0,"maximum":128},'.
  '"gateway6":{"type":"string"},'.
  '"dnsnameservers":{"type":"string"},'.
  '"dnssearch":{"type":"string"},'.
  '"mtu":{"type":["integer","string"]},'.
  '"wol":{"type":"boolean"},'.
  '"options":{"type":"string"},'.
  '"comment":{"type":"string"}';

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
		$this->registerMethod("enumerateConfiguredDevices");
		$this->registerMethod("identify");
		$this->registerMethod("getInterfaceList");
		$this->registerMethod("getInterface");
		$this->registerMethod("deleteInterface");
		$this->registerMethod("getEthernetCandidates");
		$this->registerMethod("getEthernetIface");
		$this->registerMethod("setEthernetIface");
		$this->registerMethod("enumerateBondSlaves");
		$this->registerMethod("getBondIface");
		$this->registerMethod("setBondIface");
		$this->registerMethod("getVlanCandidates");
		$this->registerMethod("getVlanIface");
		$this->registerMethod("setVlanIface");
		$this->registerMethod("getWirelessCandidates");
		$this->registerMethod("getWirelessIface");
		$this->registerMethod("setWirelessIface");
		$this->registerMethod("getProxy");
		$this->registerMethod("setProxy");
	}

	/**
	 * Helper function to get details of the given network interface device.
	 * @param deviceName The name of the network interface device.
	 * @return An array containing the network interface details. The following
	 *   fields are returned: devicename, gateway, gateway6, address, address6,
	 *   netmask, netmask6, ether, mtu, state, link, vlan and vlanid.
	 */
	private function getIfaceInfo($deviceName) {
		$obj = array();
		$obj['devicename'] = $deviceName;
		$obj['gateway'] = "";
		$obj['gateway6'] = "";
		$obj['address'] = "";
		$obj['address6'] = "";
		$obj['netmask'] = "";
		$obj['netmask6'] = -1;
		$obj['ether'] = "";
		$obj['mtu'] = "";
		$obj['state'] = gettext("UNKNOWN");
		$obj['link'] = FALSE;
		$obj['vlan'] = FALSE;
		$obj['vlanid'] = -1;
		$obj['speed'] = -1;
		$iface = OMVNetworkInterfaces::getImpl($deviceName);
		if (!is_null($iface) && $iface->exists()) {
			$gateway = $iface->getGateway();
			$gateway6 = $iface->getGateway6();
			$address = $iface->getIP();
			$address6 = $iface->getIP6();
			$netmask = $iface->getMask();
			$netmask6 = $iface->getMask6();
			$ether = $iface->getMAC();
			$state = $iface->getState();
			$speed = $iface->getSpeed();
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
			$obj['vlan'] = $iface->isVlan();
			$obj['vlanid'] = $iface->isVlan() ? $iface->getVlanId() : -1;
			$obj['speed'] = !$speed ? -1 : $speed;
		}
		return $obj;
	}

	/**
	 * Helper function to store the interface configuration object.
	 * @param params The method parameters.
	 * @param object The object containing the specific interface attributes.
	 */
    private function setInterfaceConfig($params, $object) {
        global $xmlConfig;
		// Append the generic network interface configuration attributes.
		$object = array_merge(array(
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
			"dnsnameservers" => preg_replace("/[,;]/", " ",
			  $params['dnsnameservers']),
			"dnssearch" => preg_replace("/[,;]/", " ", $params['dnssearch']),
			"mtu" => $params['mtu'],
			"wol" => array_boolval($params, 'wol'),
			"options" => $params['options'],
			"comment" => $params['comment']
		), $object);
		// Set the configuration data.
		$success = FALSE;
		if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness.
			$xpath = sprintf("//system/network/interfaces/interface".
			  "[type='%s' and devicename='%s']", $object['type'],
			  $object['devicename']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  sprintf(gettext("Network interface '%s' (type=%s) already exists"),
				  $object['devicename'], $object['type']));
			}
			// Append object to configuration.
			$success = $xmlConfig->set("//system/network/interfaces",
			  array("interface" => $object));
		} else {
			// Update existing configuration object.
			$xpath = sprintf("//system/network/interfaces/interface[".
			  "uuid='%s']", $params['uuid']);
			$success = $xmlConfig->replace($xpath, $object);
		}
		if (FALSE === $success)
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.interfaces.interface", $object);
		// Return the configuration object.
		return $object;
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
	 *   \em ether, \em mtu, \em state, \em link, \em vlan and \em vlanid.
	 * @throw E_MISC_FAILURE
	 */
	public function enumerateDevices($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Enumerate all network interface devices on the system.
		if (FALSE === ($devs = OMVNetworkInterfaces::enumerate(
		  OMV_NETWORK_INTERFACE_TYPE_ALL))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get list of network interface devices"));
		}
		// Generate the result objects including all information about
		// the network interfaces.
		$result = array();
		foreach ($devs as $devk => $devv) {
			// Get the network interface backend.
			$nib = OMVNetworkInterfaces::getBackend($devv);
			if (is_null($nib)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("No network interface backend exists for '%s'",
				  $devv));
			}
			// Set the default attributes.
			$object = array(
				"uuid" => $GLOBALS['OMV_UUID_UNDEFINED'],
				"comment" =>  "",
				"_used" =>  FALSE,
				"_readonly" =>  FALSE
			);
			// Is there any configuration for the given network interface?
			$xpath = sprintf("//system/network/interfaces/*[devicename='%s']",
			  $devv);
			$cfgObject = $xmlConfig->get($xpath);
			if (!is_null($cfgObject))
				$object = array_merge($object, $cfgObject);
			// Get the current network interface information. Note, this will
			// override various configuration values.
			$object = array_merge($object, $this->getIfaceInfo($devv));
			// Categorize the network interface.
			$typeNames = $GLOBALS['OMV_NETWORK_INTERFACE_TYPE_NAMES'];
			if (array_key_exists($nib->getType(), $typeNames)) {
				$object['type'] = $typeNames[$nib->getType()];
			} else {
				$object['type'] = "unknown";
			}
			// Do some configuration checks?
			// - Is the ethernet interface device somewhere referenced?
			$xpath = sprintf("//interfaceref[contains(.,'%s')]",
			  $object['uuid']);
			if (TRUE === $xmlConfig->exists($xpath))
				$object['_used'] = TRUE;
			// - Check if it is used by a bonding interface.
			$xpath = sprintf("//system/network/interfaces/interface[".
			  "type='bond' and contains(slaves,'%s')]", $devv);
			if (TRUE === $xmlConfig->exists($xpath)) {
				$object['_used'] = TRUE;
				$object['_readonly'] = TRUE;
				$object['method'] = "";
				$object['method6'] = "";
			}
			// Append network interface device to result list.
			$result[] = $object;
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
	 */
	function enumerateDevicesList($params, $context) {
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
		// Get the system network interface devices.
		$result = $this->callMethod("enumerateDevices", NULL, $context);
		// Filter the result.
		return $this->applyFilter($result, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get all configured network interfaces devices.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of applicable network interfaces devices.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function enumerateConfiguredDevices($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$result = array();
        // Get all network interface configuration object UUID's.
		$xpath = "//system/network/interfaces/interface/uuid";
		$uuids = $xmlConfig->getList($xpath);
		// Now get the network interface configuration objects.
		$result = array();
		foreach ($uuids as $uuidk => $uuidv) {
			// Get the configuration object.
			$object = $this->callMethod("getInterface", array(
				  "uuid" => $uuidv
			  ), $context);
			// Append some additional information, e.g. whether this
			// interface is referenced and used:
			$object['_used'] = FALSE;
			$object['_readonly'] = FALSE;
			$xpath = sprintf("//interfaceref[contains(.,'%s')]",
			$object['uuid']);
			if (TRUE === $xmlConfig->exists($xpath))
				$object['_used'] = TRUE;
			$xpath = sprintf("//interface[type='bond' and ".
			  "contains(slaves,'%s')]", $object['devicename']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				$object['_used'] = TRUE;
				$object['_readonly'] = TRUE;
			}
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
			if (0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
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
	 * Get all network interface configuration objects. Note, only the
	 * generic interface attributes are converted from a string representation
	 * to their origin value type.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function getInterfaceList($params, $context) {
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
		// Get all network interface configuration objects.
		$result = $this->callMethod("enumerateConfiguredDevices",
		  $params, $context);
		// Filter the result.
		return $this->applyFilter($result, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a network interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getInterface($params, $context) {
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
		$xpath = sprintf("//system/network/interfaces/interface[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the generic network interface configuration data.
		// Other interface type related data must be modified separately.
		$object['netmask6'] = intval($object['netmask6']);
		$object['wol'] = boolvalEx($object['wol']);
		if (!empty($object['mtu']))
			$object['mtu'] = intval($object['mtu']);
		$object['dnsnameservers'] = str_replace(" ", ",",
		  $object['dnsnameservers']);
		$object['dnssearch'] = str_replace(" ", ",", $object['dnssearch']);
		return $object;
	}

	/**
	 * Delete a network interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	function deleteInterface($params, $context) {
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
		$xpath = sprintf("//system/network/interfaces/interface[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.network.interfaces.interface");
	}

	/**
	 * Get list of devices that can be used to create an ethernet connection.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing network interface objects.
	 * @throw E_EXEC_MISC
	 */
	public function getEthernetCandidates($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get all unused network interface devices, except loopback and
		// VLAN network interfaces.
		if (FALSE === ($devs = OMVNetworkInterfaces::enumerateUnused(
		  OMV_NETWORK_INTERFACE_TYPE_ETHERNET))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get list of unused network interface devices"));
		}
		// Get the network interface information.
		$result = array();
		foreach ($devs as $devk => $devv) {
			// Does a configuration object exists for the given network
			// interface device?
			$xpath = sprintf("//system/network/interfaces/interface".
			  "[type='ethernet' and devicename='%s']", $devv);
			$object = $xmlConfig->get($xpath);
			if (!is_null($object))
				continue;
			$result[] = $this->getIfaceInfo($devv);
		}
		return $result;
	}

	/**
	 * Get a ethernet network interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getEthernetIface($params, $context) {
		// Get the configuration object.
		$object = $this->callMethod("getInterface", $params, $context);
		// Modify result data.
		return $object;
	}

	/**
	 * Set (add/update) a ethernet network interface configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setEthernetIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
                  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  '.$GLOBALS['OMV_JSONSCHEMA_NETWORK_INTERFACE_GENERIC'].'
			  }
		  }');
		// Set the configuration object.
		return $this->setInterfaceConfig($params, array(
			"type" => "ethernet"
		));
	}

	/**
	 * Enumerate all interfaces from a bonded interface object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the bonded interface configuration object.
	 *   \em unused If TRUE then the ethernet network interfaces that are
	 *   not configured nor used will be appended to the result list.
	 *   Defaults to TRUE.
	 * @param context The context of the caller.
	 * @return If \em uuid is set to OMV_UUID_UNDEFINED a list of available
	 *   ethernet network interfaces (iface class objects) is returned,
	 *   otherwise the bonded interface slaves plus the unreferenced ethernet
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
		if (array_key_exists("unused", $params) && $params['unused'] === TRUE) {
			// Get all unused network interface devices, except loopback and
			// VLAN network interfaces.
			if (FALSE === ($devs = OMVNetworkInterfaces::enumerateUnused(
			  OMV_NETWORK_INTERFACE_TYPE_ALL & ~(OMV_NETWORK_INTERFACE_TYPE_VLAN |
			  OMV_NETWORK_INTERFACE_TYPE_LOOPBACK)))) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  gettext("Failed to get list of unused network interface devices"));
			}
			// Get a list of configured ethernet network interfaces that are
			// not used by any other config object.
			foreach ($devs as $devk => $devv) {
				// Is network interface device referenced somewhere:
				// - Check if the interface is already used by a bond
				//   interface.
				$xpath = sprintf("//system/network/interfaces/interface".
				  "[type='bond' and contains(slaves,'%s')]", $devv);
				if (TRUE === $xmlConfig->exists($xpath))
					continue;
				// - Check if the interface is already used by a VLAN
				//   interface.
				$xpath = sprintf("//system/network/interfaces/interface".
				  "[type='vlan' and contains(vlanrawdevice,'%s')]", $devv);
				if (TRUE === $xmlConfig->exists($xpath))
					continue;
				// - Does a configuration object exists for the given
				//   network interface device?
				$xpath = sprintf("//system/network/interfaces/interface".
				  "[type='ethernet' and devicename='%s']", $devv);
				$object = $xmlConfig->get($xpath);
				if (!is_null($object))
					continue;
				// Get the network interface information.
				$result[] = $this->getIfaceInfo($devv);
			}
		}
		if ($params['uuid'] !== $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Get network interfaces already assigned to the bonding
			// interface device.
			$xpath = sprintf("//system/network/interfaces/interface".
			  "[uuid='%s' and type='bond']", $params['uuid']);
			$object = $xmlConfig->get($xpath);
			if (is_null($object)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Get interface details of the bonding slaves.
			foreach (explode(",", $object['slaves']) as $slavek => $slavev) {
				$result[] = $this->getIfaceInfo($slavev);
			}
			// Remove the bond interface itself from the result list.
			foreach ($result as $resultk => $resultv) {
				if ($resultv['devicename'] !== $object['devicename'])
					continue;
				// Remove the item and re-index array.
				unset($result[$resultk]);
				$result = array_values($result);
				break;
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
		// Get the configuration object.
		$object = $this->callMethod("getInterface", $params, $context);
		// Modify result data.
		$object['bondmode'] = intval($object['bondmode']);
		$object['bondmiimon'] = intval($object['bondmiimon']);
		$object['bonddowndelay'] = intval($object['bonddowndelay']);
		$object['bondupdelay'] = intval($object['bondupdelay']);
		return $object;
	}

	/**
	 * Set (add/update) a bond interface config object.
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
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
                  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  '.$GLOBALS['OMV_JSONSCHEMA_NETWORK_INTERFACE_GENERIC'].',
				  "slaves":{"type":"string","format":"regex","pattern":'.
					'"\/^((eth|wlan)\\\d+,)*(eth|wlan)\\\d+$\/i"},
				  "bondprimary":{"type":"string"},
				  "bondmode":{"type":"integer","enum":[0,1,2,3,4,5,6]},
				  "bondmiimon":{"type":"integer","minimum":0},
				  "bonddowndelay":{"type":"integer","minimum":0},
				  "bondupdelay":{"type":"integer","minimum":0}
			  }
		  }');
		// Do some tests:
		// - The primary option is only valid for active-backup(1),
		//   balance-tlb (5) and balance-alb (6) mode.
		//   See https://www.kernel.org/doc/Documentation/networking/bonding.txt
		if (empty($params['bondprimary']) && in_array($params['bondmode'],
		  array(1, 5, 6))) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_METHOD_INVALID_PARAMS,
			  "bondprimary; bondmode");
		}
		// - Check usage.
		if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check if the slaves are already used by another bond interface.
			foreach (explode(",", $params['slaves']) as $slavek => $slavev) {
				$xpath = sprintf("//system/network/interfaces/interface".
				  "[type='bond' and contains(slaves,'%s')]", $slavev);
				if (TRUE === $xmlConfig->exists($xpath)) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
					  sprintf(gettext("The interface '%s' is already used by a bonding interface"),
					  $slavev));
				}
			}
			// Get the next free device name.
			$params['devicename'] = OMVSystem::getNextDevice("iface", "bond");
		}
		// Set the configuration object.
		return $this->setInterfaceConfig($params, array(
            "type" => "bond",
			"bondprimary" => $params['bondprimary'],
			"bondmode" => $params['bondmode'],
			"bondmiimon" => $params['bondmiimon'],
			"bonddowndelay" => $params['bonddowndelay'],
			"bondupdelay" => $params['bondupdelay'],
			"slaves" => $params['slaves']
		));
	}

	/**
	 * Get list of devices that can be used to create a VLAN connection.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing network interface objects.
	 * @throw E_EXEC_MISC
	 */
	public function getVlanCandidates($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get all unused network interface devices, except loopback and
		// VLAN network interfaces.
		if (FALSE === ($devs = OMVNetworkInterfaces::enumerateUnused(
		  OMV_NETWORK_INTERFACE_TYPE_ALL & ~(OMV_NETWORK_INTERFACE_TYPE_VLAN |
		  OMV_NETWORK_INTERFACE_TYPE_LOOPBACK)))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get list of unused network interface devices"));
		}
		// Get the network interface information.
		$result = array();
		foreach ($devs as $devk => $devv) {
			$result[] = $this->getIfaceInfo($devv);
		}
		return $result;
	}

	/**
	 * Get a VLAN network interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getVlanIface($params, $context) {
		// Get the configuration object.
		$object = $this->callMethod("getInterface", $params, $context);
		// Modify result data.
		$object['vlanid'] = intval($object['vlanid']);
		return $object;
	}

	/**
	 * Set (add/update) a VLAN network interface configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setVlanIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
                  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  '.$GLOBALS['OMV_JSONSCHEMA_NETWORK_INTERFACE_GENERIC'].',
				  "vlanid":{"type":"integer","minimum":1,"maximum":4095},
				  "vlanrawdevice":{"type":"string"}
			  }
		  }');
		// Modify the method parameters.
		$params = array_merge($params, array(
			"devicename" => sprintf("%s.%d", $params['vlanrawdevice'],
			  $params['vlanid']),
		));
		// Set the configuration object.
		return $this->setInterfaceConfig($params, array(
            "type" => "vlan",
			"vlanid" => $params['vlanid'],
			"vlanrawdevice" => $params['vlanrawdevice']
		));
	}

	/**
	 * Get list of devices that can be used to create a Wi-Fi connection.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing network interface objects.
	 * @throw E_EXEC_MISC
	 */
	public function getWirelessCandidates($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get all unused network interface devices, except loopback and
		// VLAN network interfaces.
		if (FALSE === ($devs = OMVNetworkInterfaces::enumerateUnused(
		  OMV_NETWORK_INTERFACE_TYPE_WIRELESS))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get list of unused network interface devices"));
		}
		// Get the network interface information.
		$result = array();
		foreach ($devs as $devk => $devv) {
			// Does a configuration object exists for the given network
			// interface device?
			$xpath = sprintf("//system/network/interfaces/interface".
			  "[type='wireless' and devicename='%s']", $devv);
			$object = $xmlConfig->get($xpath);
			if (!is_null($object))
				continue;
			$result[] = $this->getIfaceInfo($devv);
		}
		return $result;
	}

	/**
	 * Get a wireless network interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
    function getWirelessIface($params, $context) {
		// Get the configuration object.
		$object = $this->callMethod("getInterface", $params, $context);
		// Modify result data.
		return $object;
	}

	/**
	 * Set (add/update) a wireless network interface configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
    function setWirelessIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
                  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  '.$GLOBALS['OMV_JSONSCHEMA_NETWORK_INTERFACE_GENERIC'].',
				  "wpassid":{"type":"string"},
				  "wpapsk":{"type":"string"}
			  }
		  }');
		// Set the configuration object.
		return $this->setInterfaceConfig($params, array(
            "type" => "wireless",
			"wpassid" => $params['wpassid'],
			"wpapsk" => $params['wpapsk']
		));
    }

	/**
	 * Get the proxy settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getProxy($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the network proxy settings.
		$xpath = "//system/network/proxy";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return array(
			"httpenable" => boolvalEx($object['http']['enable']),
			"httphost" => $object['http']['host'],
			"httpport" => intval($object['http']['port']),
			"httpusername" => $object['http']['username'],
			"httppassword" => $object['http']['password'],
			"httpsenable" => boolvalEx($object['https']['enable']),
			"httpshost" => $object['https']['host'],
			"httpsport" => intval($object['https']['port']),
			"httpsusername" => $object['https']['username'],
			"httpspassword" => $object['https']['password'],
			"ftpenable" => boolvalEx($object['ftp']['enable']),
			"ftphost" => $object['ftp']['host'],
			"ftpport" => intval($object['ftp']['port']),
			"ftpusername" => $object['ftp']['username'],
			"ftppassword" => $object['ftp']['password']
		);
	}

	/**
	 * Set the proxy settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setProxy($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "httpenable":{"type":"boolean"},
				  "httphost":{"type":"string"},
				  "httpport":{"type":"integer","minimum":1,"maximum":65535},
				  "httpusername":{"type":"string"},
				  "httppassword":{"type":"string"},
				  "httpsenable":{"type":"boolean"},
				  "httpshost":{"type":"string"},
				  "httpsport":{"type":"integer","minimum":1,"maximum":65535},
				  "httpsusername":{"type":"string"},
				  "httpspassword":{"type":"string"},
				  "ftpenable":{"type":"boolean"},
				  "ftphost":{"type":"string"},
				  "ftpport":{"type":"integer","minimum":1,"maximum":65535},
				  "ftpusername":{"type":"string"},
				  "ftppassword":{"type":"string"}
			  }
		  }');
		// Get existing configuration object.
		$xpath = "//system/network/proxy";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the configuration object.
		$object = array_merge($object, array(
			"http" => array(
				"enable" => $params['httpenable'],
				"host" => $params['httphost'],
				"port" => $params['httpport'],
				"username" => $params['httpusername'],
				"password" => $params['httppassword']
			),
			"https" => array(
				"enable" => $params['httpsenable'],
				"host" => $params['httpshost'],
				"port" => $params['httpsport'],
				"username" => $params['httpsusername'],
				"password" => $params['httpspassword']
			),
			"ftp" => array(
				"enable" => $params['ftpenable'],
				"host" => $params['ftphost'],
				"port" => $params['ftpport'],
				"username" => $params['ftpusername'],
				"password" => $params['ftppassword']
			)
		));
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.proxy", $object);
		// Return the configuration object.
		return $object;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceNetwork());
