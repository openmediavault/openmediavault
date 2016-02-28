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
require_once("openmediavault/util.inc");
require_once("openmediavault/object.inc");

/**
 * @ingroup api
 * Control the systemd system and service manager.
 */
class OMVSystemCtl extends OMVObject {
	private $name = "";
	private $options = array();

	/**
	 * Constructor
	 * @param name The name of the unit, e.g. ssh.
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Set a command option.
	 */
	public function setOption($arg) {
		$this->options[] = $arg;
	}

	/**
	 * Reset the list of command options.
	 */
	public function clearOptions() {
		$this->options = array();
	}

	/**
	 * Execute the given command.
	 * @param command The command to execute, e.g. start, stop, restart, ...
	 * @param options A list of optional command options.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return The exit code if \em quite is set to TRUE.
	 */
	private function exec($command, $options = null, $quiet = FALSE) {
		$cmdArgs = array();
		// Append (global) command options.
		if (!empty($this->options))
			$cmdArgs = array_merge($cmdArgs, $this->options);
		// Append optional command options.
		if (!empty($options))
			$cmdArgs = array_merge($cmdArgs, $options);
		$cmdArgs[] = $command;
		$cmdArgs[] = $this->name;
		$cmd = sprintf("export LANG=C; systemctl %s 2>&1",
		  implode(" ", $cmdArgs));
		OMVUtil::exec($cmd, $output, $result);
		if (!$quiet && ($result !== 0)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		return $result;
	}

	/**
	 * Enable the specified unit file.
	 * @param now Set to TRUE to start the unit after it has been enabled.
	 *   Defaults to FALSE.
	 */
	public function enable($now = FALSE, $quiet = FALSE) {
/* The option '--now' is not supported in Debian Jessie.
		$options = array();
		if (TRUE === $now)
			$options[] = "--now";
		$this->exec("enable", $options, $quiet);
*/
		$this->exec("enable", null, $quiet);
		if (TRUE === $now)
			$this->exec("start", null, $quiet);
	}

	/**
	 * Disable the specified unit file.
	 * @param now Set to TRUE to stop the unit after it has been disabled.
	 *   Defaults to FALSE.
	 */
	public function disable($now = FALSE, $quiet = FALSE) {
/* The option '--now' is not supported in Debian Jessie.
		$options = array();
		if (TRUE === $now)
			$options[] = "--now";
		$this->exec("disable", $options, $quiet);
*/
		$this->exec("disable", null, $quiet);
		if (TRUE === $now)
			$this->exec("stop", null, $quiet);
	}

	/**
	 * Reenable the specified unit file.
	 */
	public function reenable($quiet = FALSE) {
		$this->exec("reenable", null, $quiet);
	}

	/**
	 * Checks whether the specified unit file is enabled.
	 * @return TRUE if the unit is enabled, otherwise FALSE.
	 */
	public function isEnabled() {
		return (0 == $this->exec("is-enabled", null, TRUE));
	}

	/**
	 * Start (activate) the specified unit.
	 */
	public function start($quiet = FALSE) {
		$this->exec("start", null, $quiet);
	}

	/**
	 * Stop (deactivate) the specified unit.
	 */
	public function stop($quiet = FALSE) {
		$this->exec("stop", null, $quiet);
	}

	/**
	 * Restart the specified unit.
	 */
	public function restart($quiet = FALSE) {
		$this->exec("restart", null, $quiet);
	}

	/**
	 * Asks the specified unit to reload its configuration.
	 */
	public function reload($quiet = FALSE) {
		$this->exec("reload", null, $quiet);
	}

	/**
	 * Check whether the specified unit is active (i.e. running).
	 * @return TRUE if the unit is active, otherwise FALSE.
	 */
	public function isActive() {
		return (0 == $this->exec("is-active", null, TRUE));
	}

	/**
	 * Send a signal to the process of the unit.
	 * @param signal Must be one of the well known signal specifiers such as
	 *   SIGHUP, SIGTERM, SIGINT or SIGSTOP.
	 */
	public function kill($signal, $quiet = FALSE) {
		$options = array();
		$options[] = sprintf("--signal=%s", $signal);
		return (0 == $this->exec("kill", $options, $quiet));
	}
}
