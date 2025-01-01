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
namespace OMV\System;

/**
 * @ingroup api
 */
class Monit {
	private $name = "";
	private $group = FALSE;

	/**
	 * Constructor
	 * @param name The name of the service.
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Set the group option. If set, then the methods start, stop, restart,
	 * monitor and unmonitor will be applied to the given group name.
	 * @param value Set to TRUE to use the group option, otherwise FALSE.
	 * @return void
	 */
	final public function setGroup($value) {
		$this->group = $value;
	}

	/**
	 * Start the named service and enable monitoring for it.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return void
	 */
	public function start($quiet = FALSE) {
		$this->action("start", $quiet);
	}

	/**
	 * Stop the named service and disable its monitoring.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return void
	 */
	public function stop($quiet = FALSE) {
		$this->action("stop", $quiet);
	}

	/**
	 * Restart the named service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return void
	 */
	public function restart($quiet = FALSE) {
		$this->action("restart", $quiet);
	}

	/**
	 * Enable monitoring of the named service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return void
	 */
	public function monitor($quiet = FALSE) {
		$this->action("monitor", $quiet);
	}

	/**
	 * Disable monitoring of the named service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return void
	 */
	public function unmonitor($quiet = FALSE) {
		$this->action("unmonitor", $quiet);
	}

	/**
	 * Execute the given action.
	 * @param action The action to execute, e.g. start, stop, restart, ...
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return void
	 */
	private function action($action, $quiet = FALSE) {
		$cmdArgs = [];
		// Append various command options.
		if (TRUE === $this->group) {
			$cmdArgs[] = "-g";
			$cmdArgs[] = $this->name;
			$cmdArgs[] = $action;
		} else {
			$cmdArgs[] = $action;
			$cmdArgs[] = $this->name;
		}
		// Execute the command.
		$cmd = new \OMV\System\Process("monit", $cmdArgs);
		$cmd->setQuiet($quiet);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Get the status of a monitored process.
	 * @return The status of the monitored service, e.g. 'running',
	 *   'does not exist' or 'not monitored'.
	 */
	public function status() {
		$status = "unknown";
		// Get status summary of monitored services.
		$cmd = new \OMV\System\Process(["monit", "-B", "summary"]);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Parse the command output.
		// Example:
		// Monit 5.20.0 uptime: 6m
		//  Service Name              Status                      Type
		//  omv4stretch               Running                     System
		//  rrdcached                 Running                     Process
		//  php5-fpm                  Execution failed | Does...  Process
		//  nginx                     Running                     Process
		//  omv-engined               Not monitored               Process
		//  collectd                  Execution failed | Does...  Process
		//  rootfs                    Accessible                  Filesystem
		foreach ($output as $outputk => $outputv) {
			$regex = '/^\s+([\S]+)\s+(.+)\s+Process$/i';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			if ($matches[1] !== $this->name)
				continue;
			$status = trim($matches[2]);
			break;
		}
		return mb_strtolower($status);
	}

	/**
	 * Check whether the specified service is running.
	 * @return TRUE if the service is running, otherwise FALSE.
	 */
	public function isRunning() {
		return ("running" == $this->status());
	}
}
