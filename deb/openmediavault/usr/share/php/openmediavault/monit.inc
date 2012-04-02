<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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

class OMVMonit extends OMVObject {
	/**
	 * Start the named service and enable monitoring for it.
	 * @param name The name of the service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return None
	 */
	public function start($name, $quiet = FALSE) {
		$this->action("start", $name, $quiet);
	}

	/**
	 * Stop the named service and disable its monitoring.
	 * @param name The name of the service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return None
	 */
	public function stop($name, $quiet = FALSE) {
		$this->action("stop", $name, $quiet);
	}

	/**
	 * Restart the named service.
	 * @param name The name of the service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return None
	 */
	public function restart($name, $quiet = FALSE) {
		$this->action("restart", $name, $quiet);
	}

	/**
	 * Enable monitoring of the named service.
	 * @param name The name of the service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return None
	 */
	public function monitor($name, $quiet = FALSE) {
		$this->action("monitor", $name, $quiet);
	}

	/**
	 * Disable monitoring of the named service.
	 * @param name The name of the service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return None
	 */
	public function unmonitor($name, $quiet = FALSE) {
		$this->action("unmonitor", $name, $quiet);
	}

	/**
	 * Execute the given action.
	 * @param action The action to execute, e.g. start, stop, restart, ...
	 * @param name The name of the service.
	 * @param quiet Do not throw an error on failure. Defaults to FALSE.
	 * @return None
	 */
	private function action($action, $name, $quiet = FALSE) {
		$cmd = sprintf("sudo monit %s %s 2>&1", $action, $name);
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
	 * Get the status of a monitored process.
	 * @param name The name of the service.
	 * @return The status of the monitored service, e.g. 'running'.
	 */
	public function status($name) {
		$cmd = sprintf("export LANG=C; sudo monit status | sed -e " .
		  "'/^.*%s.*$/,/^$/!d' | grep -e '^\s*status' || true", $name);
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
