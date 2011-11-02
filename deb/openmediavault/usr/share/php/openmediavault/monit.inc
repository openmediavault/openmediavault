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
require_once("openmediavault/globals.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");

class OMVMonitService extends OMVObject {
	private $name = "";
	private $enable = FALSE;

	/**
	 * Constructor
	 * @param name The name is a service entry name from the monitrc file.
	 * @param enable TRUE to start the process.
	 */
	public function __construct($name, $enable = FALSE) {
		$this->name = $name;
		$this->enable = $enable;
	}

	public function exec() {
		// Try to stop the service. Do not throw an exception in case of an
		// error returned by monit.
		$this->stop(TRUE);
		// Remove or create the services monit configuration file and start
		// the service if necessary.
		switch ($this->enable) {
		case FALSE:
			$this->update(TRUE);
			break;
		case TRUE:
			$this->update(FALSE);
			// Delay 2 second to give monit enough time to reload its
			// configuration, otherwise monit does not realize the start
			// command for the given service.
			usleep(2000000);
			$this->start();
			break;
		}
	}

	/**
	 * Start the named service and enable monitoring for it.
	 */
	public function start() {
		$this->action("start");
	}

	/**
	 * Stop the named service and disable its monitoring.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return None
	 */
	public function stop($quiet = FALSE) {
		$this->action("stop", $quiet);
	}

	/**
	 * Restart the named service.
	 */
	public function restart() {
		$this->action("restart");
	}

	/**
	 * Reinitialize a running monit daemon, the daemon will reread its
	 * configuration, close and reopen log files.
	 */
	public function reload() {
		$cmd = "sudo monit reload 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Execute the given action.
	 * @param action The action to execute, e.g. start, stop, restart, ...
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return None
	 */
	private function action($action, $quiet = FALSE) {
		$cmd = sprintf("sudo monit %s %s 2>&1", $action, $this->name);
		OMVUtil::exec($cmd, $output, $result);
		if (!$quiet && ($result !== 0)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	private function update($remove) {
		switch ($remove) {
		case FALSE:
			// Create the monit service configuration file
			$cmd = sprintf("sudo omv-monit-service enable %s", $this->name);
			break;
		case TRUE:
			// Remove the monit service configuration file
			$cmd = sprintf("sudo omv-monit-service disable %s", $this->name);
			break;
		}
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Force monit to reload its configuration
		$this->reload();
	}

	/**
	 * Get the status of the monitored process.
	 * @return The status of the monitored service, e.g. 'running'.
	 */
	public function status() {
		$cmd = sprintf("export LANG=C; sudo monit status | sed -e " .
		  "'/^.*%s.*$/,/^$/!d' | grep -e '^\s*status' || true",
		  $this->name);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$result = "unknown";
		if (isset($output[0])) {
			$result = trim(str_replace("status", "", $output[0]));
		}
		return $result;
	}
}
?>
