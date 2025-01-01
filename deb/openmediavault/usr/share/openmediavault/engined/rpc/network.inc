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
namespace Engined\Rpc;

require_once("openmediavault/functions.inc");

class Network extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getInformation");
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
		$this->registerMethod("enumerateBridgeSlaves");
		$this->registerMethod("getBridgeIface");
		$this->registerMethod("setBridgeIface");
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
		// Set the default values.
		$obj = [
			"devicename" => $deviceName,
			"gateway" => "",
			"gateway6" => "",
			"address" => "",
			"address6" => "",
			"prefix" => -1,
			"prefix6" => -1,
			"netmask" => "",
			"netmask6" => "",
			"ether" => "",
			"mtu" => "",
			"state" => gettext("UNKNOWN"),
			"link" => FALSE,
			"vlan" => FALSE,
			"vlanid" => -1,
			"speed" => -1,
			"description" => "",
			"stats" => []
		];
		// Get the network interface object.
		$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
		$iface = $mngr->getImpl($deviceName);
		if (!is_null($iface) && $iface->exists()) {
			$gateway = $iface->getGateway();
			$gateway6 = $iface->getGateway6();
			$address = $iface->getIP();
			$address6 = $iface->getIP6();
			$prefix = $iface->getPrefix();
			$prefix6 = $iface->getPrefix6();
			$netmask = $iface->getNetmask();
			$netmask6 = $iface->getNetmask6();
			$ether = $iface->getMAC();
			$state = $iface->getState();
			$speed = $iface->getSpeed();
			$stats = $iface->getStatistics();
			$obj['type'] = $iface->getType();
			$obj['description'] = $iface->getDescription();
			$obj['stats'] = !$stats ? [] : $stats;
			$obj['gateway'] = !$gateway ? "" : $gateway;
			$obj['gateway6'] = !$gateway6 ? "" : $gateway6;
			$obj['address'] = !$address ? "" : $address;
			$obj['address6'] = !$address6 ? "" : $address6;
			$obj['prefix'] = !$prefix ? -1 : $prefix;
			$obj['prefix6'] = !$prefix6 ? -1 : $prefix6;
			$obj['netmask'] = !$netmask ? "" : $netmask;
			$obj['netmask6'] = !$netmask6 ? "" : $netmask6;
			$obj['ether'] = !$ether ? "" : $ether;
			$obj['mtu'] = $iface->getMTU();
			$obj['state'] = !$state ? gettext("UNKNOWN") : $state;
			$obj['link'] = $iface->getLink();
			$obj['speed'] = !$speed ? -1 : $speed;
			if ("vlan" == $obj['type']) {
				$obj['vlan'] = TRUE;
				$obj['vlanid'] = $iface->getVlanId();
			}
		}
		return $obj;
	}

	/**
	 * Helper function to store the interface configuration object.
	 * @param params The method parameters.
	 * @param ifaceParams The object containing the specific interface
	 *   attributes.
	 * @return The configuration object.
	 */
    private function setInterfaceConfig($params, $ifaceParams) {
		// Modify various configuration attributes.
		$params['dnsnameservers'] = implode(" ", explode_csv(
		    $params['dnsnameservers']));
		$params['dnssearch'] = implode(" ", explode_csv(
		    $params['dnssearch']));
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.network.interface");
		$object->setAssoc($params);
		$object->setAssoc($ifaceParams);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			// Check uniqueness.
			$db->assertIsUniqueByFilter($object, [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "type",
					"arg1" => $object->get("type")
				],
				"arg1" => [
					"operator" => "stringEquals",
					"arg0" => "devicename",
					"arg1" => $object->get("devicename")
				]
			]);
		}
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get general network settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	function getGeneralSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.network.dns");
		return $object->getAssoc();
	}

	/**
	 * Set general network settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setGeneralSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.network.setgeneralsettings");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.network.dns");
		$object->setAssoc($params);
		// Update the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Enumerate all network interface devices on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a network interface
	 *   device with the following properties: \em devicename, \em gateway,
	 *   \em gateway6, \em address, \em address6, \em netmask, \em netmask6,
	 *   \em ether, \em mtu, \em state, \em link, \em vlan and \em vlanid.
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
		// Enumerate all network interface devices on the system.
		if (FALSE === ($devs = $mngr->enumerate(
		  OMV_NETWORK_INTERFACE_TYPE_ALL))) {
			throw new \OMV\Exception(
			  "Failed to get list of network interface devices.");
		}
		// Generate the result objects including all information about
		// the network interfaces.
		$result = [];
		foreach ($devs as $devk => $devv) {
			// Get the network interface backend.
			$mngr->assertBackendExists($devv);
			$nib = $mngr->getBackend($devv);
			// Set the default attributes.
			$object = [
				"uuid" => \OMV\Environment::get("OMV_CONFIGOBJECT_NEW_UUID"),
				"comment" =>  "",
				"_used" =>  FALSE,
				"_readonly" =>  FALSE
			];
			// Is there any configuration for the given network interface?
			$filter = [
				"operator" => "stringEquals",
				"arg0" => "devicename",
				"arg1" => $devv
			];
			$db = \OMV\Config\Database::getInstance();
			if ($db->exists("conf.system.network.interface", $filter)) {
				// Get the interface configuration object.
				$cfgObject = $db->getByFilter("conf.system.network.interface",
				  $filter, 1);
				// Append the configuration attributes.
				$object = array_merge($object, $cfgObject->getAssoc());
				$object['slaves'] = explode_safe(",", $object['slaves']);
				// Is the interface device somewhere referenced?
				$object['_used'] = $db->isReferenced($cfgObject);
			}
			// Get the current network interface information. Note, this will
			// override various configuration values.
			$object = array_merge($object, $this->getIfaceInfo($devv));
			// Check whether the interface device is used by a bonding
			// interface.
			if ($db->exists("conf.system.network.interface", [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "type",
					"arg1" => "bond"
				],
				"arg1" => [
					"operator" => "stringContains",
					"arg0" => "slaves",
					"arg1" => $devv
				]
			])) {
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
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
	 */
	function enumerateConfiguredDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get all network interface configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.network.interface");
		// Now get the network interface configuration objects.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			$objectAssoc = $objectv->getAssoc();
			// Is the ethernet interface device somewhere referenced?
			$objectAssoc['_used'] = $db->isReferenced($objectv);
			$objectAssoc['_readonly'] = FALSE;
			// Check if it is used by a bonding interface.
			if ($db->exists("conf.system.network.interface", [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "type",
					"arg1" => "bond"
				],
				"arg1" => [
					"operator" => "stringContains",
					"arg0" => "slaves",
					"arg1" => $objectv->get("devicename")
				]
			])) {
				$objectAssoc['_used'] = TRUE;
				$objectAssoc['_readonly'] = TRUE;
			}
			$objectAssoc['slaves'] = explode_safe(",", $objectAssoc['slaves']);
			$result[] = $objectAssoc;
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
	 */
	function identify($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.network.identify");
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// Initiate adapter-specific action intended to enable an operator
			// to easily identify the adapter by sight.
			$cmdArgs = [];
			$cmdArgs[] = "--identify";
			$cmdArgs[] = escapeshellarg($params['devicename']);
			$cmdArgs[] = $params['seconds'];
			$cmd = new \OMV\System\Process("ethtool", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}

	/**
	 * Get runtime information about the given network interface.
	 * @param params An array containing the following fields:
	 *   \em devicename The name of the network interface device, e.g. ens6.
	 * @param context The context of the caller.
	 * @return An array containing the network interface information.
	 */
	function getInformation($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.network.getInformation");
		return $this->getIfaceInfo($params['devicename']);
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
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
	 */
	function getInterface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.network.interface", $params['uuid']);
		// Modify the generic network interface configuration data.
		// Other interface type related data must be modified separately.
		$result = $object->getAssoc();
		$result['dnsnameservers'] = str_replace(" ", ",",
			$result['dnsnameservers']);
		$result['dnssearch'] = str_replace(" ", ",", $result['dnssearch']);
		$result['slaves'] = explode_safe(",", $result['slaves']);
		return $result;
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.network.interface", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get list of devices that can be used to create an ethernet connection.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing network interface objects.
	 */
	public function getEthernetCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
		// Get all unused ethernet network interface devices.
		if (FALSE === ($devs = $mngr->enumerateUnused(
				OMV_NETWORK_INTERFACE_TYPE_ETHERNET))) {
			throw new \OMV\Exception(
				"Failed to get list of network interface devices.");
		}
		// Process the identified devices and skip those that are
		// already configured or in use.
		$result = [];
		foreach ($devs as $devk => $devv) {
			$db = \OMV\Config\Database::getInstance();
			if ($db->exists("conf.system.network.interface", [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "type",
					"arg1" => "ethernet"
				],
				"arg1" => [
					"operator" => "stringEquals",
					"arg0" => "devicename",
					"arg1" => $devv
				]
			])) {
				continue;
			}
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
	 */
	function getEthernetIface($params, $context) {
		return $this->callMethod("getInterface", $params, $context);
	}

	/**
	 * Set (add/update) a ethernet network interface configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setEthernetIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, "rpc.network.setethernetiface");
		// Set the configuration object.
		return $this->setInterfaceConfig($params, [
			"type" => "ethernet"
		]);
	}

	/**
	 * Enumerate all interfaces from a bonded interface object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the bonded interface configuration object.
	 *   \em unused If TRUE then the network interfaces that are
	 *   not configured nor used will be appended to the result list.
	 *   Defaults to TRUE.
	 * @param context The context of the caller.
	 * @return If \em uuid is set to OMV_CONFIGOBJECT_NEW_UUID a list of
	 *   available network interfaces (iface class objects) is returned,
	 *   otherwise the bonded interface slaves and the unreferenced
	 *   network interfaces are returned.
	 */
	public function enumerateBondSlaves($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.network.enumeratebondslaves");
		// Get list of network interfaces.
		$db = \OMV\Config\Database::getInstance();
		$result = [];
		if (array_value($params, "unused", FALSE) === TRUE) {
			// Get all existing ethernet network interface devices.
			$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
			if (FALSE === ($devs = $mngr->enumerate(
					OMV_NETWORK_INTERFACE_TYPE_ETHERNET))) {
				throw new \OMV\Exception(
					"Failed to get list of network interface devices.");
			}
			// Process the identified devices and skip those that are
			// already configured or in use.
			foreach ($devs as $devk => $devv) {
				// Is network interface device referenced somewhere:
				// - Check if the interface is already used by a bond
				//   or bridge interface.
				if ($db->exists("conf.system.network.interface", [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEnum",
						"arg0" => "type",
						"arg1" => [ "bond", "bridge" ]
					],
					"arg1" => [
						"operator" => "stringContains",
						"arg0" => "slaves",
						"arg1" => $devv
					]
				])) {
					continue;
				}
				// - Check if the interface is already used by a VLAN
				//   interface.
				if ($db->exists("conf.system.network.interface", [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEquals",
						"arg0" => "type",
						"arg1" => "vlan"
					],
					"arg1" => [
						"operator" => "stringContains",
						"arg0" => "vlanrawdevice",
						"arg1" => $devv
					]
				])) {
					continue;
				}
				// - Does a configuration object exists for the given
				//   network interface device?
				if ($db->exists("conf.system.network.interface", [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEquals",
						"arg0" => "type",
						"arg1" => "ethernet"
					],
					"arg1" => [
						"operator" => "stringEquals",
						"arg0" => "devicename",
						"arg1" => $devv
					]
				])) {
					continue;
				}
				// Get the network interface information.
				$result[] = $this->getIfaceInfo($devv);
			}
		}
		if ($params['uuid'] !== \OMV\Environment::get(
		  "OMV_CONFIGOBJECT_NEW_UUID")) {
			// Get network interfaces already assigned to the bonding
			// interface device.
			$object = $db->getByFilter("conf.system.network.interface", [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "uuid",
					"arg1" => $params['uuid']
				],
				"arg1" => [
					"operator" => "stringEquals",
					"arg0" => "type",
					"arg1" => "bond"
				]
			], 1);
			// Get interface details of the bonding slaves.
			foreach (explode_safe(",", $object->get("slaves")) as $slavek => $slavev) {
				$result[] = $this->getIfaceInfo($slavev);
			}
			// Remove the bond interface itself from the result list.
			foreach ($result as $resultk => $resultv) {
				if ($resultv['devicename'] !== $object->get("devicename")) {
					continue;
				}
				// Remove the item and re-index the array.
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
	 */
	function getBondIface($params, $context) {
		return $this->callMethod("getInterface", $params, $context);
	}

	/**
	 * Set (add/update) a bond interface config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setBondIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, "rpc.network.setbondiface");
		// Do some tests:
		// - The primary option is only valid for active-backup(1),
		//   balance-tlb (5) and balance-alb (6) mode.
		//   See https://www.kernel.org/doc/Documentation/networking/bonding.txt
		if (empty($params['bondprimary']) && in_array($params['bondmode'],
		  [ 1, 5, 6 ])) {
			throw new \OMV\Exception(
			  "The parameters 'bondprimary' and 'bondmode' are invalid.");
		}
		// - Check usage.
		if ($params['uuid'] == \OMV\Environment::get(
		  "OMV_CONFIGOBJECT_NEW_UUID")) {
			// Check if the slaves are already used by another bond interface.
			foreach ($params['slaves'] as $slavek => $slavev) {
				$db = \OMV\Config\Database::getInstance();
				if ($db->exists("conf.system.network.interface", [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEquals",
						"arg0" => "type",
						"arg1" => "bond"
					],
					"arg1" => [
						"operator" => "stringContains",
						"arg0" => "slaves",
						"arg1" => $slavev
					]
				])) {
					throw new \OMV\Exception(sprintf("The interface '%s' is ".
					  "already used by a bonding interface.", $slavev));
				}
			}
			// Get the next available device name.
			$params['devicename'] = \OMV\System\Net\NetworkInterfaceBond::
				getNextDeviceName();
		}
		// Set the configuration object.
		$params['slaves'] = implode(",", $params['slaves']);
		return $this->setInterfaceConfig($params, [
			"type" => "bond",
			"bondprimary" => $params['bondprimary'],
			"bondmode" => $params['bondmode'],
			"bondmiimon" => $params['bondmiimon'],
			"bonddowndelay" => $params['bonddowndelay'],
			"bondupdelay" => $params['bondupdelay'],
			"bondtransmithashpolicy" => $params['bondtransmithashpolicy'],
			"slaves" => $params['slaves']
		]);
	}

	/**
	 * Get list of devices that can be used to create a VLAN connection.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing network interface objects.
	 */
	public function getVlanCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get all configured network interfaces except VLANs.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.network.interface", [
			"operator" => "not",
			"arg0" => [
				"operator" => "stringEquals",
				"arg0" => "type",
				"arg1" => "vlan"
			]
		]);
		// Get the network interface information.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			$result[] = $this->getIfaceInfo($objectv->get("devicename"));
		}
		return $result;
	}

	/**
	 * Get a VLAN network interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getVlanIface($params, $context) {
		return $this->callMethod("getInterface", $params, $context);
	}

	/**
	 * Set (add/update) a VLAN network interface configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setVlanIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, "rpc.network.setvlaniface");
		// Modify the method parameters.
		$params = array_merge($params, [
			"devicename" => sprintf("%s.%d", $params['vlanrawdevice'],
			  $params['vlanid'])
		]);
		// Set the configuration object.
		return $this->setInterfaceConfig($params, [
			"type" => "vlan",
			"vlanid" => $params['vlanid'],
			"vlanrawdevice" => $params['vlanrawdevice']
		]);
	}

	/**
	 * Get list of devices that can be used to create a Wi-Fi connection.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing network interface objects.
	 */
	public function getWirelessCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get all unused wifi network interface devices.
		$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
		if (FALSE === ($devs = $mngr->enumerateUnused(
				OMV_NETWORK_INTERFACE_TYPE_WIFI))) {
			throw new \OMV\Exception(
				"Failed to get list of network interface devices.");
		}
		// Process the identified devices and skip those that are
		// already configured or in use.
		$result = [];
		foreach ($devs as $devk => $devv) {
			$db = \OMV\Config\Database::getInstance();
			if ($db->exists("conf.system.network.interface", [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "type",
					"arg1" => "wifi"
				],
				"arg1" => [
					"operator" => "stringEquals",
					"arg0" => "devicename",
					"arg1" => $devv
				]
			])) {
				continue;
			}
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
	 */
    function getWirelessIface($params, $context) {
		return $this->callMethod("getInterface", $params, $context);
	}

	/**
	 * Set (add/update) a wireless network interface configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
    function setWirelessIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, "rpc.network.setwirelessiface");
		// Set the configuration object.
		return $this->setInterfaceConfig($params, [
			"type" => "wifi",
			"wpassid" => $params['wpassid'],
			"wpapsk" => $params['wpapsk'],
			"keymanagement" => $params['keymanagement'],
			"hidden" => $params['hidden']
		]);
    }

	/**
	 * Enumerate all interfaces from a bridge interface object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the bridge interface configuration object.
	 *   \em unused If TRUE then the network interfaces that are
	 *   not configured nor used will be appended to the result list.
	 *   Defaults to TRUE.
	 * @param context The context of the caller.
	 * @return If \em uuid is set to OMV_CONFIGOBJECT_NEW_UUID a list of
	 *   available network interfaces (iface class objects) is returned,
	 *   otherwise the bonded interface slaves and the unreferenced
	 *   network interfaces are returned.
	 */
	public function enumerateBridgeSlaves($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.network.enumeratebridgeslaves");
		// Get list of network interfaces.
		$db = \OMV\Config\Database::getInstance();
		$result = [];
		if (array_value($params, "unused", FALSE) === TRUE) {
			// Get all unused network interface devices, except bridge,
			// wifi and loopback network interfaces.
			$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
			if (FALSE === ($devs = $mngr->enumerateUnused(
					OMV_NETWORK_INTERFACE_TYPE_ALL &
					~(OMV_NETWORK_INTERFACE_TYPE_BRIDGE |
					OMV_NETWORK_INTERFACE_TYPE_WIFI |
					OMV_NETWORK_INTERFACE_TYPE_LOOPBACK)))) {
				throw new \OMV\Exception(
					"Failed to get list of network interface devices.");
			}
			// Process the identified devices and skip those that are
			// already configured or in use.
			foreach ($devs as $devk => $devv) {
				// Is network interface device referenced somewhere:
				// - Check if the interface is already used by a bond
				//   or bridge interface.
				if ($db->exists("conf.system.network.interface", [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEnum",
						"arg0" => "type",
						"arg1" => [ "bond", "bridge" ]
					],
					"arg1" => [
						"operator" => "stringContains",
						"arg0" => "slaves",
						"arg1" => $devv
					]
				])) {
					continue;
				}
				// - Check if the interface is already used by a VLAN
				//   interface.
				if ($db->exists("conf.system.network.interface", [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEquals",
						"arg0" => "type",
						"arg1" => "vlan"
					],
					"arg1" => [
						"operator" => "stringContains",
						"arg0" => "vlanrawdevice",
						"arg1" => $devv
					]
				])) {
					continue;
				}
				// - Does a configuration object exists for the given
				//   network interface device?
				if ($db->exists("conf.system.network.interface", [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEquals",
						"arg0" => "type",
						"arg1" => "ethernet"
					],
					"arg1" => [
						"operator" => "stringEquals",
						"arg0" => "devicename",
						"arg1" => $devv
					]
				])) {
					continue;
				}
				// Get the network interface information.
				$result[] = $this->getIfaceInfo($devv);
			}
		}
		if ($params['uuid'] !== \OMV\Environment::get(
		  "OMV_CONFIGOBJECT_NEW_UUID")) {
			// Get network interfaces already assigned to the bridge
			// interface device.
			$object = $db->getByFilter("conf.system.network.interface", [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "uuid",
					"arg1" => $params['uuid']
				],
				"arg1" => [
					"operator" => "stringEquals",
					"arg0" => "type",
					"arg1" => "bridge"
				]
			], 1);
			// Get interface details of the bridge slaves.
			foreach (explode_safe(",", $object->get("slaves")) as $slavek => $slavev) {
				$result[] = $this->getIfaceInfo($slavev);
			}
		}
		return $result;
	}

	/**
	 * Get a bridge interface configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getBridgeIface($params, $context) {
		return $this->callMethod("getInterface", $params, $context);
	}

	/**
	 * Set (add/update) a bridge interface config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setBridgeIface($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the network interface specific parameters of the
		// RPC service method.
		$this->validateMethodParams($params, "rpc.network.setbridgeiface");
		// Do some tests:
		// - Check usage.
		if ($params['uuid'] == \OMV\Environment::get(
		  "OMV_CONFIGOBJECT_NEW_UUID")) {
			// Check if the slaves are already used by another bridge interface.
			foreach ($params['slaves'] as $slavek => $slavev) {
				$db = \OMV\Config\Database::getInstance();
				if ($db->exists("conf.system.network.interface", [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEquals",
						"arg0" => "type",
						"arg1" => "bridge"
					],
					"arg1" => [
						"operator" => "stringContains",
						"arg0" => "slaves",
						"arg1" => $slavev
					]
				])) {
					throw new \OMV\Exception(sprintf("The interface '%s' is ".
					  "already used by a bridge interface.", $slavev));
				}
			}
			// Get the next available device name.
			$params['devicename'] = \OMV\System\Net\NetworkInterfaceBridge::
				getNextDeviceName();
		}
		// Set the configuration object.
		$params['slaves'] = implode(",", $params['slaves']);
		return $this->setInterfaceConfig($params, [
			"type" => "bridge",
			"slaves" => $params['slaves']
		]);
	}

	/**
	 * Get the proxy settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	function getProxy($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the network proxy settings.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.network.proxy");
		$object->move("http.enable", "httpenable");
		$object->move("http.host", "httphost");
		$object->move("http.port", "httpport");
		$object->move("http.username", "httpusername");
		$object->move("http.password", "httppassword");
		$object->move("https.enable", "httpsenable");
		$object->move("https.host", "httpshost");
		$object->move("https.port", "httpsport");
		$object->move("https.username", "httpsusername");
		$object->move("https.password", "httpspassword");
		$object->move("ftp.enable", "ftpenable");
		$object->move("ftp.host", "ftphost");
		$object->move("ftp.port", "ftpport");
		$object->move("ftp.username", "ftpusername");
		$object->move("ftp.password", "ftppassword");
		return $object->getAssoc();
	}

	/**
	 * Set the proxy settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setProxy($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.network.setproxy");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.network.proxy");
		$object->setAssoc([
			"http" => [
				"enable" => $params['httpenable'],
				"host" => $params['httphost'],
				"port" => $params['httpport'],
				"username" => $params['httpusername'],
				"password" => $params['httppassword']
			],
			"https" => [
				"enable" => $params['httpsenable'],
				"host" => $params['httpshost'],
				"port" => $params['httpsport'],
				"username" => $params['httpsusername'],
				"password" => $params['httpspassword']
			],
			"ftp" => [
				"enable" => $params['ftpenable'],
				"host" => $params['ftphost'],
				"port" => $params['ftpport'],
				"username" => $params['ftpusername'],
				"password" => $params['ftppassword']
			]
		]);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}
}
