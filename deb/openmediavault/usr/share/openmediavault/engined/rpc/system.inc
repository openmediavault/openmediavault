<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
class OMVRpcServiceSystem extends \OMV\Rpc\ServiceAbstract {
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
	 */
	public function noop($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		return array_rand_value(\OMV\Environment::get("OMV_DUNE_QUOTES"));
	}

	/**
	 * Get the list of running processes and a system summary information
	 * as well.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The 'top' console command output.
	 */
	public function getTopInfo($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get task overview.
		$cmd = new \OMV\System\Process("top", "-b", "-n 1");
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	/**
	 * Get a list of available shells.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array with the available shells.
	 */
	public function getShells($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Prepare result list.
		$result = [
			"/bin/false",
			"/usr/sbin/nologin"
		];
		// Get list of available shells.
		$shells = file("/etc/shells");
		foreach ($shells as $shellk => $shellv) {
			$shellv = trim($shellv);
			if (is_executable($shellv))
				$result[] = $shellv;
		}
		return $result;
	}

	/**
	 * Reboot the system.
	 * @param params An array containing the following fields:
	 *   \em delay The number of seconds to delay.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function reboot($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.system.reboot");
		// Delay execution?
		if (0 < $params['delay']) {
			$pid = $this->fork();
			if (0 < $pid) { // Parent process.
				// Exit immediatelly to do not block the caller (e.g. WebGUI).
				return;
			}
			// Child process.
			sleep($params['delay']);
		}
		$pm = new \OMV\System\PowerManagement();
		$pm->reboot();
	}

	/**
	 * Shut down the system.
	 * @param params An array containing the following fields:
	 *   \em delay The number of seconds to delay.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function shutdown($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.system.shutdown");
		// Delay execution?
		if (0 < $params['delay']) {
			$pid = $this->fork();
			if (0 < $pid) { // Parent process.
				// Exit immediatelly to do not block the caller (e.g. WebGUI).
				return;
			}
			// Child process.
			sleep($params['delay']);
		}
		$pm = new \OMV\System\PowerManagement();
		$pm->shutdown();
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
	 * @param params An array containing the following fields:
	 *   \em delay The number of seconds to delay.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function standby($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.system.standby");
		// Delay execution?
		if (0 < $params['delay']) {
			$pid = $this->fork();
			if (0 < $pid) { // Parent process.
				// Exit immediatelly to do not block the caller (e.g. WebGUI).
				return;
			}
			// Child process.
			sleep($params['delay']);
		}
		$pm = new \OMV\System\PowerManagement();
		$pm->standby();
	}

	/**
	 * Put the machine into suspend to RAM (STR) mode. If this state is not
	 * supported the system will be shut down.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function suspend($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$pm = new \OMV\System\PowerManagement();
		$pm->suspend();
	}

	/**
	 * Put the machine into suspend to disk (STD) mode. If this state is not
	 * supported the system will be shut down.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function hibernate($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$pm = new \OMV\System\PowerManagement();
		$pm->hibernate();
	}

	/**
	 * Get system time settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function getTimeSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.time");
		// Prepare the result values.
		return [
			"date" => [
				"local" => strftime("%c"),
				"ISO8601" => date("c")
			],
			"timezone" => $object->get("timezone"),
			"ntpenable" => $object->get("ntp.enable"),
			"ntptimeservers" => $object->get("ntp.timeservers")
		];
	}

	/**
	 * Set system time settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw \OMV\Exception
	 */
	public function setTimeSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.system.settimesettings");
		// Validate the time zone.
		$tzList = $this->callMethod("getTimeZoneList", NULL, $context);
		if (FALSE === in_array($params['timezone'], $tzList)) {
			throw new \OMV\Exception("Invalid time zone '%s'",
				$params['timezone']);
		}
		// Get and update the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.time");
		$object->set("timezone", $params['timezone']);
		$object->set("ntp.enable", $params['ntpenable']);
		$object->set("ntp.timeservers", $params['ntptimeservers']);
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Set the system date.
	 * @param params An array containing the following fields:
	 *   \em timestamp The date to set as UNIX timestamp.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function setDate($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.system.setdate");
		// Set the system date.
		$cmdArgs = [];
		$cmdArgs[] = sprintf("--set='@%d'", $params['timestamp']);
		$cmd = new \OMV\System\Process("date", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Set the system date via NTP.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function setNtpDate($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Set the system date via NTP.
		$cmdArgs = [];
		$cmdArgs[] = "-g";
		$cmdArgs[] = "-q";
		$cmd = new \OMV\System\Process("ntpd", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Get a list of time zones.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array with the time zone strings.
	 */
	public function getTimeZoneList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// According to http://php.net/manual/en/timezones.others.php
		// some of the time zones that the Debian installer supports are
		// deprectated. To prevent errors we support them to (ALL_WITH_BC).
		$timezoneIdentifiers = \DateTimeZone::listIdentifiers(
			\DateTimeZone::ALL_WITH_BC);
		sort($timezoneIdentifiers, SORT_NATURAL);
		return $timezoneIdentifiers;
	}

	/**
	 * Get various system information.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of system information.
	 */
	public function getInformation($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get various system informations.
		$prd = new \OMV\ProductInfo();
		$uname = posix_uname();
		$memStats = \OMV\System\System::getMemoryStats();
		$memStats['wobufferscache']['percentage'] = round(
		  ($memStats['mem']['used'] * 100) / $memStats['mem']['total']);
		$cpuStats = \OMV\System\System::getCpuStats();
		$index = 0;
		return [
			[
				"name" => gettext("Hostname"),
				"value" => \OMV\System\Net\Dns::getFqdn(),
				"type" => "string",
				"index" => $index++
			],[
				"name" => gettext("Version"),
				"value" => sprintf("%s (%s)", $prd->getVersion(),
				  $prd->getVersionName()),
				"type" => "string",
				"index" => $index++
			],[
				"name" => gettext("Processor"),
				"value" => $cpuStats['modelname'],
				"type" => "string",
				"index" => $index++
			],[
				"name" => gettext("Kernel"),
				"value" => sprintf("%s %s", $uname['sysname'],
				  $uname['release']),
				"type" => "string",
				"index" => $index++
			],[
				"name" => gettext("System time"),
				"value" => strftime("%c"),
				"type" => "string",
				"index" => $index++
			],[
				"name" => gettext("Uptime"),
				"value" => \OMV\System\System::uptime(),
				"type" => "string",
				"index" => $index++
			],[
				"name" => gettext("Load average"),
				"value" => \OMV\System\System::getLoadAverage(),
				"type" => "string",
				"index" => $index++
			],[
				"name" => gettext("CPU usage"),
				"value" => [
					"text" => sprintf("%d%%", $cpuStats['usage']),
//					"text" => sprintf("%d%% of %s MHz", $cpuStats['usage'],
//					  $cpuStats['cpumhz']),
					"value" => $cpuStats['usage']
				],
				"type" => "progress",
				"index" => $index++
			],[
				"name" => gettext("Memory usage"),
				"value" => [
					"text" => sprintf("%d%% of %s",
					  $memStats['wobufferscache']['percentage'],
					  binary_format($memStats['mem']['total'])),
					"value" => $memStats['wobufferscache']['percentage']
				],
				"type" => "progress",
				"index" => $index++
			]
		];
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$cmd = new \OMV\System\Process("omv-sysinfo");
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}
}
