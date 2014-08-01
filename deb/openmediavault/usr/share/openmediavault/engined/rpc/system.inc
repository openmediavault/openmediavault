<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/system.inc");
require_once("openmediavault/product.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceSystem extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "System";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("noop");
		$this->registerMethod("getTopInfo");
		$this->registerMethod("getShells");
		$this->registerMethod("reboot");
		$this->registerMethod("shutdown");
		$this->registerMethod("standby");
		$this->registerMethod("suspend");
		$this->registerMethod("hibernate");
		$this->registerMethod("getTimeSettings");
		$this->registerMethod("setTimeSettings");
		$this->registerMethod("setDate");
		$this->registerMethod("setNtpDate");
		$this->registerMethod("getTimeZoneList");
		$this->registerMethod("getInformation");
		$this->registerMethod("getDiagnosticReport");
	}

	/**
	 * This function can be used to check if the communication between
	 * WebGUI and server exists.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A quotation from the Dune novel, which is a string.
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function noop($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
			  "will remain.",
			"People always fear the unknown, if they are truthful about ".
			  "it. But i can face my fears."
		);
		return $result[mt_rand(0, count($result) - 1)];
	}

	/**
	 * Get the list of running processes and a system summary information
	 * as well.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The 'top' console command output.
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function getTopInfo($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get task overview.
		OMVUtil::exec("top -b -n 1", $result);
		return implode("\n", $result);
	}

	/**
	 * Get a list of available shells.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array with the available shells.
	 * @throw E_EXEC_FAILED
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function getShells($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Prepare result list.
		$result = array(
			"/bin/false",
			"/usr/sbin/nologin"
		);
		// Get list of available shells.
		$shells = file("/etc/shells");
		foreach($shells as $shellk => $shellv) {
			$shellv = trim($shellv);
			if(is_executable($shellv)) {
				$result[] = $shellv;
			}
		}
		return $result;
	}

	/**
	 * Reboot the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @throw E_EXEC_FAILED
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function reboot($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Reboot the system now.
		$cmd = "export LANG=C; shutdown -r now 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Shut down the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function shutdown($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Shutdown the system now.
		$cmd = "export LANG=C; shutdown -h -P now 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Put the machine in a sleep state. If suspend to disk or RAM is not
	 * supported the system will be shut down. The system will be put into
	 * one of the following state depending on which state is supported: <ul>
	 * \li Hybrid suspend (disk and RAM)
	 * \li Suspend to disk
	 * \li Suspend to RAM
	 * \li Shut down and turn of system
	 * </ul>
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function standby($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$pm = new OMVPowerManagement();
		if (FALSE === $pm->standby()) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  $pm->getLastError());
		}
	}

	/**
	 * Put the machine into suspend to RAM (STR) mode. If this state is not
	 * supported the system will be shut down.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function suspend($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$pm = new OMVPowerManagement();
		if (FALSE === $pm->suspend()) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  $pm->getLastError());
		}
	}

	/**
	 * Put the machine into suspend to disk (STD) mode. If this state is not
	 * supported the system will be shut down.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_MISC
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function hibernate($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// 
		$pm = new OMVPowerManagement();
		if (FALSE === $pm->hibernate()) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  $pm->getLastError());
		}
	}

	/**
	 * Get system time settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_EXEC_MISC
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function getTimeSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get configuration object.
		$xpath = "//system/time";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return array(
			"date" => array(
				"local" => strftime("%c"),
				"ISO8601" => date("c")
			),
			"timezone" => $object['timezone'],
			"ntpenable" => boolvalEx($object['ntp']['enable']),
			"ntptimeservers" => $object['ntp']['timeservers']
		);
	}

	/**
	 * Set system time settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 * @throw E_RPC_SERVICE_METHOD_INVALID_PARAMS
	 */
	public function setTimeSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "timezone":{"type":"string"},
				  "ntpenable":{"type":"boolean"},
				  "ntptimeservers":{"type":"string"}
			  }
		  }');
		// Prepare configuration data.
		$object = array(
			"timezone" => $params['timezone'],
			"ntp" => array(
				"enable" => array_boolval($params, 'ntpenable'),
				"timeservers" => $params['ntptimeservers']
			)
		);
		// Set configuration object.
		if(FALSE === $xmlConfig->replace("//system/time", $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.time", $object);
	}

	/**
	 * Set the system date.
	 * @param params An array containing the following fields:
	 *   \em timestamp The date to set as UNIX timestamp.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 * @throw E_RPC_SERVICE_METHOD_INVALID_PARAMS
	 */
	public function setDate($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "timestamp":{"type":"integer"}
			  }
		  }');
		// Set the system date.
		$cmd = sprintf("date --set='@%d' 2>&1", $params['timestamp']);
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Set the system date via NTP.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function setNtpDate($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Set the system date via NTP.
		$cmd = "ntpd -gq 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Get a list of time zones.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array with the time zone strings.
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function getTimeZoneList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Prepare list of time zone identifiers (for ComboBox store).
		$timezoneIdentifiers = DateTimeZone::listIdentifiers();
		natsort($timezoneIdentifiers);
		return $timezoneIdentifiers;
	}

	/**
	 * Get various system informations.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of system informations.
	 * @throw E_EXEC_MISC
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	public function getInformation($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get various system informations.
		$prd = new OMVProduct();
		$uname = posix_uname();
		$memStats = OMVSystem::getMemoryStats();
		$memStats['wobufferscache']['percentage'] = round(
		  ($memStats['wobufferscache']['used'] * 100) /
		  $memStats['mem']['total']);
		$cpuStats = OMVSystem::getCPUStats();
		$index = 0;
		return array(
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
				"value" => strftime("%c"),
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
					"text" => sprintf("%d%% of %s",
					  $memStats['wobufferscache']['percentage'],
					  binary_format($memStats['mem']['total'])),
					"value" => $memStats['wobufferscache']['percentage']
				),
				"type" => "progress",
				"index" => $index++)
		);
	}

	/**
	 * Get the 'omv-sysinfo' diagnostic report.
	 * @param params The method parameters.
	 *   The method does not have any paramaters.
	 * @param context The context of the caller.
	 * @return A quotation from the Dune novel, which is a string.
	 */
	public function getDiagnosticReport($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		OMVUtil::exec("omv-sysinfo", $result);
		return implode("\n", $result);
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSystem());
?>
