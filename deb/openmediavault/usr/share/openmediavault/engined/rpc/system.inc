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

class System extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getTimeZoneList");
		$this->registerMethod("getInformation");
		$this->registerMethod("getDiagnosticReport");
		$this->registerMethod("getCpuStats");
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
		$cmd = new \OMV\System\Process("top", "--batch",
			"--iterations=1", "--width=512");
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
		// Prepare the result list.
		$result = [
			"/usr/bin/false",
			"/usr/sbin/nologin"
		];
		// Get list of available shells.
		$shells = file("/etc/shells");
		foreach ($shells as $shellk => $shellv) {
			$shellv = trim($shellv);
			if (is_executable($shellv)) {
				$result[] = $shellv;
			}
		}
		// Filter duplicates. Remove `/bin/xxx` if `/usr/bin/xxx` exists.
		foreach ($result as $shellk => $shellv) {
			foreach (['bin', 'sbin'] as $origin) {
				if (str_starts_with($shellv, sprintf("/%s/", $origin))) {
					$shellPath = sprintf("/usr/%s/%s", $origin,
						basename($shellv));
					if (in_array($shellPath, $result)) {
						unset($result[$shellk]);
					}
				}
			}
		}
		sort($result, SORT_NATURAL);
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
				// Exit immediately to do not block the caller (e.g. WebGUI).
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
				// Exit immediately to do not block the caller (e.g. WebGUI).
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
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->getAssoc("conf.system.powermngmnt");
		// Delay execution?
		if (0 < $params['delay']) {
			$pid = $this->fork();
			if (0 < $pid) { // Parent process.
				// Exit immediately to do not block the caller (e.g. WebGUI).
				return;
			}
			// Child process.
			sleep($params['delay']);
		}
		$pm = new \OMV\System\PowerManagement();
		switch ($object['standbymode']) {
		case "poweroff":
			$pm->shutdown();
			break;
		case "suspend":
			$pm->suspend();
			break;
		case "hibernate":
			$pm->hibernate();
			break;
		case "suspendhybrid":
			$pm->suspendHybrid();
			break;
		}
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
			"ntptimeservers" => $object->get("ntp.timeservers"),
			"ntpclients" => $object->get("ntp.clients")
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
		$object->set("ntp.clients", $params['ntpclients']);
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
		// deprecated. To prevent errors we support them to (ALL_WITH_BC).
		$timezoneIdentifiers = \DateTimeZone::listIdentifiers(
			\DateTimeZone::ALL_WITH_BC);
		sort($timezoneIdentifiers, SORT_NATURAL);
		return $timezoneIdentifiers;
	}

	/**
	 * Get various system information.
	 * Note, all numbers that might be > 4GiB are returned as strings
	 * to keep the 32bit compatibility.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An object containing the system information.
	 */
	public function getInformation($params, $context) {
		// The information everyone is allowed to see.
		$result = [
			"ts" => time(),
			"time" => strftime("%c"),
			"hostname" => \OMV\System\Net\Dns::getFqdn()
		];
		try {
			$pkgUpgradeList = \OMV\System\System::getAptUpgradeList();
		} catch(\Exception $e) {
			$pkgUpgradeList = [];
		}
		// Append the information that is only visible to an administrator.
		if (TRUE === $this->hasContextRole($context, OMV_ROLE_ADMINISTRATOR)) {
			$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
			$prd = new \OMV\ProductInfo();
			$uname = posix_uname();
			$memStats = \OMV\System\System::getMemoryStats();
			$cpuStats = \OMV\System\System::getCpuStats();
			$dirtyModules = $moduleMngr->getDirtyModulesAssoc();
			$result = array_merge($result, [
				"version" => sprintf("%s (%s)", $prd->getVersion(),
					$prd->getVersionName()),
				"cpuModelName" => $cpuStats['modelname'],
				"cpuUtilization" => $cpuStats['utilization'],
				"cpuCores" => $cpuStats['cores'],
				"cpuMhz" => $cpuStats['mhz'],
				"memTotal" => $memStats['mem']['total'],
				"memFree" => $memStats['mem']['free'],
				"memUsed" => $memStats['mem']['used'],
				"memAvailable" => $memStats['mem']['available'],
				"memUtilization" => $memStats['mem']['utilization'],
				"kernel" => sprintf("%s %s", $uname['sysname'],
					$uname['release']),
				"uptime" => \OMV\System\System::uptime(),
				"loadAverage" => \OMV\System\System::getLoadAverage(),
				"configDirty" => !empty($dirtyModules),
				"dirtyModules" => $dirtyModules,
				"rebootRequired" => file_exists("/run/reboot-required"),
				"availablePkgUpdates" => count($pkgUpgradeList),
				"displayWelcomeMessage" => file_exists(
					"/var/lib/openmediavault/display_welcome_message")
			]);
		}
		return $result;
	}

	/**
	 * Get the 'omv-sysinfo' diagnostic report.
	 * @param params The method parameters.
	 *   The method does not have any paramaters.
	 * @param context The context of the caller.
	 * @return string Returns the diagnostic report as plain text.
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

	/**
	 * Get CPU statistics.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An object containing the CPU statistics.
	 */
	public function getCpuStats($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$result = \OMV\System\System::getCpuStats();
		return $result;
	}
}
