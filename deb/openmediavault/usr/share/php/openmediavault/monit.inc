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

	/**
	 * Constructor
	 * @param name The name is a service entry name from the monitrc file.
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Start the named service and enable monitoring for it.
	 */
	public function start($quiet = FALSE) {
		$this->action("start", $quiet);
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
	public function restart($quiet = FALSE) {
		$this->action("restart", $quiet);
	}

	/**
	 * Enable monitoring of the named service.
	 */
	public function monitor($quiet = FALSE) {
		$this->action("monitor", $quiet);
	}

	/**
	 * Disable monitoring of the named service.
	 */
	public function unmonitor($quiet = FALSE) {
		$this->action("unmonitor", $quiet);
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

	/**
	 * Create the monit configuration and reinitialize monit.
	 */
	public function update() {
		// Create monit and monitored services configuration files.
		$cmd = "sudo omv-mkconf monit";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Reinitialize monit.
		$cmd = "sudo monit reload";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
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
