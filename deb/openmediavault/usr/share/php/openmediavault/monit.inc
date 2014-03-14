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
require_once("openmediavault/globals.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");

/**
 * @ingroup api
 */
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
		$cmd = sprintf("export LANG=C; monit %s %s 2>&1", $action, $name);
		OMVUtil::exec($cmd, $output, $result);
		if(!$quiet && ($result !== 0)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create the monit configuration and reinitialize monit.
	 */
	public function update() {
		// Create monit and monitored services configuration files.
		$cmd = "omv-mkconf monit 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Reinitialize monit.
		$cmd = "export LANG=C; monit reload 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
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
		$status = "unknown";
		// Get status summary of monitored services. 
		$cmd = "export LANG=C; monit summary";
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Parse the command output.
		foreach($output as $outputk => $outputv) {
			$regex = '/^Process\s+\'([^\']+)\'\s+(.+)$/i';
			if(1 !== preg_match($regex, $outputv, $matches))
				continue;
			if($matches[1] != $name)
				continue;
			$status = trim($matches[2]);
			break;
		}
		return $status;
	}
}
?>
