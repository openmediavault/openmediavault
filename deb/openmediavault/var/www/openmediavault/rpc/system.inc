<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/product.inc");
require_once("openmediavault/notify.inc");

class SystemRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setTimeSettings" => array('{
				"type":"object",
				"properties":{
					"timezone":{"type":"string"},
					"ntpenable":{"type":"boolean"},
					"ntptimeservers":{"type":"string"}
				}
			}'),
			"setDate" => array(
				'{"type":"integer"}'
			),
			"setWebUISettings" => array('{
				"type":"object",
				"properties":{
					"port":{"type":"integer","minimum":1,"maximum":65535},
					"enablessl":{"type":"boolean"},
					"sslport":{"type":"integer","minimum":1,"maximum":65535},
					"forcesslonly":{"type":"boolean"},
					"sslcertificateref":{'.
					  $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'},
					"timeout":{"type":"integer","minimum":1,"maximum":30}
				}
			}')
		);
	}

	/**
	 * This function can be used to check if the communication between
	 * WebGUI and server exists.
	 * @return None.
	 */
	function noop() {
		$this->commitSession();
		// http://www.nightsolo.net/dune/god.html
		$result = array(
			"The beginning of knowledge is the discovery of something ".
			  "we do not understand.",
			"He who controls the spice controls the universe.",
			"Try looking into that place where you dare not look! You'll ".
			  "find me there, staring out at you!",
			"A large populace held in check by a small but powerful force ".
			  "is quite a common situation in our universe. And we know the ".
			  "major conditions wherein this large populace may turn upon ".
			  "its keepers: When they find a leader. This is the most ".
			  "volatile threat to the powerful; they must retain control of ".
			  "leaders. When the populace recognizes its chains. Keep the ".
			  "populace blind and unquestioning. When the populace perceives ".
			  "a hope of escape from bondage. They must never even believe ".
			  "that escape is possible!",
			"I must not fear. Fear is the mind-killer. Fear is the ".
			  "little-death that brings total obliteration. I will face my ".
			  "fear. I will permit it to pass over me and through me. And ".
			  "when it has gone past I will turn the inner eye to see its ".
			  "path. Where the fear has gone there will be nothing. Only I ".
			  "will remain."
		);
		return $result[mt_rand(0, count($result))];
	}

	/**
	 * Get the list of running processes and a system summary information
	 * as well.
	 * @return The 'top' console command output.
	 */
	function getTopInfo() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get task overview
		OMVUtil::exec("top -b -n 1", $result);
		return implode("\n", $result);
	}

	/**
	 * Get a list of available shells.
	 * @return The list of available shells.
	 */
	function getShells() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of available shells
		$cmd = 'for sh in $(sudo cat /etc/shells); do '.
		  'if [ -x ${sh} ]; then echo ${sh}; fi done';
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$data = array(
			array("path" => "/bin/false"),
			array("path" => "/usr/sbin/nologin")
		);
		sort($output);
		foreach ($output as $path) {
			$data[] = array(
				"path" => $path
			);
		}
		return array(
			"total" => count($data),
			"data" => $data
		);
	}

	/**
	 * Get a list of system users (from /etc/passwd)
	 * @return The list of users.
	 */
	function getSystemUsers() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of system users
		$users = OMVSystemUsers::get();
		foreach ($users as $userk => $userv) {
			$data[] = array(
				"name" => $userv
			);
		}
		$result = array(
			"total" => count($data),
			"data" => $data
		);
		return $result;
	}

	/**
	 * Reboot the system.
	 */
	function reboot() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Reboot the system now
		$cmd = "sudo shutdown -r now 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Shutdown the system.
	 */
	function shutdown() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Shutdown the system now
		$cmd = "sudo shutdown -h -P now 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Get system time settings.
	 */
	function getTimeSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//system/time";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get local date/time
		if (FALSE === ($date = OMVSystem::date())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  gettext("Failed to get system date and time"));
		}
		return array(
			"date" => array(
				"local" => $date['local'],
				"ISO8601" => $date['ISO8601']
			),
			"timezone" => $object['timezone'],
			"ntpenable" => boolval($object['ntp']['enable']),
			"ntptimeservers" => $object['ntp']['timeservers']
		);
	}

	/**
	 * Set system time settings.
	 */
	function setTimeSettings($data) {
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
			"timezone" => $data['timezone'],
			"ntp" => array(
				"enable" => array_boolval($data, 'ntpenable'),
				"timeservers" => $data['ntptimeservers']
			)
		);
		// Set configuration object
		if (FALSE === $xmlConfig->replace("//system/time", $object)) {
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
		  "org.openmediavault.system.time", $object);
	}

	/**
	 * Set system date.
	 * @param date The date to set as UNIX timestamp.
	 */
	function setDate($data) {
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
		// Set the date
		$cmd = sprintf("sudo date --set='@%d' 2>&1", $data);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Set system date via NTP.
	 */
	function setNtpDate() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Set the date
		$cmd = "sudo ntpd -gq 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Get list of time zones.
	 */
	function getTimeZoneList() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Prepare list of time zone identifiers (for ComboBox store)
		$timezoneIdentifiers = DateTimeZone::listIdentifiers();
		natsort($timezoneIdentifiers);
		$data = array();
		foreach ($timezoneIdentifiers as $timezoneIdentifier) {
			$data[] = array(
				"value" => $timezoneIdentifier
			);
		}
		return array(
			"total" => count($data),
			"data" => $data
		);
	}

	/**
	 * Get WebUI settings.
	 */
	function getWebUISettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//webadmin";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['port'] = intval($object['port']);
		$object['enablessl'] = boolval($object['enablessl']);
		$object['sslport'] = intval($object['sslport']);
		$object['forcesslonly'] = boolval($object['forcesslonly']);
		$object['timeout'] = intval($object['timeout']);
		return $object;
	}

	/**
	 * Set WebUI settings.
	 */
	function setWebUISettings($data) {
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
		// Prepare configuration objects
		$object = array(
			"port" => $data['port'],
			"enablessl" => array_boolval($data, 'enablessl'),
			"sslport" => $data['sslport'],
			"forcesslonly" => array_boolval($data, 'forcesslonly'),
			"sslcertificateref" => $data['sslcertificateref'],
			"timeout" => $data['timeout']
		);
		// Set configuration object
		$xpath = "//webadmin";
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
		  "org.openmediavault.webadmin", $object);
	}

	/**
	 * Set general system informations.
	 */
	function getInformation() {
		$prd = new OMVProduct();
		$uname = posix_uname();
		$memStats = OMVSystem::getMemoryStats();
		$cpuStats = OMVSystem::getCPUStats();
		if (FALSE === ($date = OMVSystem::date())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  gettext("Failed to get system date and time"));
		}
		$index = 0;
		$data = array(
			array(
				"name" => gettext("Hostname"),
				"value" => OMVNetwork::getHostname("--fqdn"),
				"type" => "string",
				"index" => $index++),
			array(
				"name" => gettext("Version"),
				"value" => sprintf("%s (%s)", $prd->getVersion(),
				  $prd->getVersionName()),
				"type" => "string",
				"index" => $index++),
			array(
				"name" => gettext("Processor"),
				"value" => $cpuStats['modelname'],
				"type" => "string",
				"index" => $index++),
			array(
				"name" => gettext("Kernel"),
				"value" => sprintf("%s %s", $uname['sysname'],
				  $uname['release']),
				"type" => "string",
				"index" => $index++),
			array(
				"name" => gettext("System time"),
				"value" => $date['local'],
				"type" => "string",
				"index" => $index++),
			array(
				"name" => gettext("Uptime"),
				"value" => OMVSystem::uptime(),
				"type" => "string",
				"index" => $index++),
			array(
				"name" => gettext("Load average"),
				"value" => OMVSystem::getLoadAverage(),
				"type" => "string",
				"index" => $index++),
			array(
				"name" => gettext("CPU usage"),
				"value" => array(
					"text" => sprintf("%d%%", $cpuStats['usage']),
					"value" => $cpuStats['usage']
				),
				"type" => "progress",
				"index" => $index++),
			array(
				"name" => gettext("Memory usage"),
				"value" => array(
					"text" => sprintf("%d%% of %s", $memStats['percent'],
					  binary_format($memStats['total'])),
					"value" => $memStats['percent']
				),
				"type" => "progress",
				"index" => $index++)
		);
		$result = array(
			"total" => count($data),
			"data" => $data
		);
		return $result;
	}
}
?>
