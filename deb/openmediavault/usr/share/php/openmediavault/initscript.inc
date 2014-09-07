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
require_once("openmediavault/util.inc");
require_once("openmediavault/object.inc");

/**
 * @ingroup api
 */
class OMVSysVInitScript extends OMVObject {
	private $name = "";
	private $enable = FALSE;
	private $update = TRUE;
	private $delay = 1000000; // 1 second

	/**
	 * Constructor
	 * @param name The name of the init script.
	 * @param enable TRUE to start the process. Defaults to FALSE.
	 * @param update TRUE to call the update-rc.d script. Defaults to TRUE.
	 */
	public function __construct($name, $enable = FALSE, $update = TRUE) {
		$this->name = $name;
		$this->enable = $enable;
		$this->update = $update;
	}

	/**
	 * Depending on the given constriuctor parameter do the following:
	 * 1. Stop the init script
	 * 2. If service is enabled, then install and start init script,
	 *    otherwise remove init script.
	 */
	public function exec() {
		$this->stop();
		switch($this->enable) {
		case FALSE:
			$this->update(TRUE);
			break;
		case TRUE:
			$this->update(FALSE);
			usleep($this->delay);
			$this->start();
			break;
		}
	}

	/**
	 * Enable or remove a system init script.
	 * @param disable Set TRUE to remove the system init script.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 */
	private function update($disable) {
		if(!$this->update)
			return;
		$cmdargs = array();
		switch($disable) {
		case FALSE:
			$cmdargs[] = escapeshellarg($this->name);
			$cmdargs[] = "enable";
			break;
		case TRUE:
			$cmdargs[] = escapeshellarg($this->name);
			$cmdargs[] = "disable";
			break;
		}
		$cmd = sprintf("export LANG=C; update-rc.d %s 2>&1", implode(
		  " ", $cmdargs));
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Enable the system init script.
	 */
	public function enable() {
		$this->update(FALSE);
	}

	/**
	 * Disable the system init script.
	 */
	public function disable() {
		$this->update(TRUE);
	}

	/**
	 * Make links to start and stop the service in the defined runlevels.
	 * @throw E_EXEC_FAILED
	 */
	public function defaults() {
		$cmd = sprintf("export LANG=C; update-rc.d %s defaults 2>&1",
		  escapeshellarg($this->name));
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Remove the system init script from all runlevels.
	 * @throw E_EXEC_FAILED
	 */
	public function remove() {
		$cmd = sprintf("export LANG=C; update-rc.d --force %s ".
		  "remove 2>&1", escapeshellarg($this->name));
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Invoke init script with parameter 'start'.
	 */
	public function start() {
		$this->invoke("start");
	}

	/**
	 * Invoke init script with parameter 'stop'.
	 */
	public function stop() {
		$this->invoke("stop");
	}

	/**
	 * Invoke init script with parameter 'restart'.
	 */
	public function restart() {
		$this->invoke("restart");
	}

	/**
	 * Invoke init script with parameter 'reload'.
	 */
	public function reload() {
		$this->invoke("reload");
	}

	/**
	 * Invoke init script with the given aktion. Throws an exception in case
	 * of an error.
	 * @param action The action to be executed.
	 * @throw E_EXEC_FAILED
	 */
	public function invoke($action) {
		$cmd = sprintf("export LANG=C; invoke-rc.d %s %s 2>&1",
		  escapeshellarg($this->name), $action);
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Invoke init script with parameter 'status'.
	 * @return One of the following exit status codes:
	 * 0        program is running or service is OK
	 * 1        program is dead and /var/run pid file exists
	 * 2        program is dead and /var/lock lock file exists
	 * 3        program is not running
	 * 4        program or service status is unknown
	 * 5-99     reserved for future LSB use
	 * 100-149  reserved for distribution use
	 * 150-199  reserved for application use
	 * 200-254  reserved
	 * See http://refspecs.freestandards.org/LSB_3.1.0/LSB-Core-generic/LSB-Core-generic/iniscrptact.html
	 */
	public function status() {
		$cmd = sprintf("export LANG=C; invoke-rc.d %s status 2>&1",
		  escapeshellarg($this->name));
		OMVUtil::exec($cmd, $output, $result);
		return $result;
	}
}
?>
