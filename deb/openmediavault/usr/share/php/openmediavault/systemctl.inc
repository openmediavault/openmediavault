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
	private $cmdArgs = [];

	/**
	 * Constructor
	 * @param name The name of the unit, e.g. ssh.
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Set an optional command line argument.
	 */
	public function setCmdArg($arg) {
		$this->cmdArgs[] = $arg;
	}

	/**
	 * Execute the given command.
	 * @param command The command to execute, e.g. start, stop, restart, ...
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return The exit code if \em quite is set to TRUE.
	 */
	private function exec($command, $quiet = FALSE) {
		$cmdArgs = array();
		if (!empty($this->cmdArgs))
			$cmdArgs = array_merge($cmdArgs, $this->cmdArgs);
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
		if (TRUE === $now)
			$this->setCmdArg("--now");
		$this->exec("enable", $quiet);
*/
		$this->exec("enable", $quiet);
		if (TRUE === $now)
			$this->exec("start", $quiet);
	}

	/**
	 * Disable the specified unit file.
	 * @param now Set to TRUE to stop the unit after it has been disabled.
	 *   Defaults to FALSE.
	 */
	public function disable($now = FALSE, $quiet = FALSE) {
/* The option '--now' is not supported in Debian Jessie.
		if (TRUE === $now)
			$this->setCmdArg("--now");
		$this->exec("disable", $quiet);
*/
		$this->exec("disable", $quiet);
		if (TRUE === $now)
			$this->exec("stop", $quiet);
	}

	/**
	 * Reenable the specified unit file.
	 */
	public function reenable($quiet = FALSE) {
		$this->exec("reenable", $quiet);
	}

	/**
	 * Checks whether the specified unit file is enabled.
	 * @return TRUE if the unit is enabled, otherwise FALSE.
	 */
	public function isEnabled() {
		return (0 == $this->exec("is-enabled", TRUE));
	}

	/**
	 * Start (activate) the specified unit.
	 */
	public function start($quiet = FALSE) {
		$this->exec("start", $quiet);
	}

	/**
	 * Stop (deactivate) the specified unit.
	 */
	public function stop($quiet = FALSE) {
		$this->exec("stop", $quiet);
	}

	/**
	 * Restart the specified unit.
	 */
	public function restart($quiet = FALSE) {
		$this->exec("restart", $quiet);
	}

	/**
	 * Asks the specified unit to reload its configuration.
	 */
	public function reload($quiet = FALSE) {
		$this->exec("reload", $quiet);
	}

	/**
	 * Check whether the specified unit is active (i.e. running).
	 * @return TRUE if the unit is active, otherwise FALSE.
	 */
	public function isActive() {
		return (0 == $this->exec("is-active", TRUE));
	}
}
