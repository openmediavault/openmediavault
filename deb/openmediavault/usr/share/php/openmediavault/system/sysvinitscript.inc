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
class SysVInitScript {
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
		switch ($this->enable) {
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
	 * @return void
	 */
	private function update($disable) {
		if (!$this->update)
			return;
		$cmdArgs = [];
		switch ($disable) {
		case FALSE:
			$cmdArgs[] = escapeshellarg($this->name);
			$cmdArgs[] = "enable";
			break;
		case TRUE:
			$cmdArgs[] = escapeshellarg($this->name);
			$cmdArgs[] = "disable";
			break;
		}
		$cmd = new \OMV\System\Process("update-rc.d", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
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
	 */
	public function defaults() {
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->name);
		$cmdArgs[] = "defaults";
		$cmd = new \OMV\System\Process("update-rc.d", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Remove the system init script from all runlevels.
	 */
	public function remove() {
		$cmdArgs = [];
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($this->name);
		$cmdArgs[] = "remove";
		$cmd = new \OMV\System\Process("update-rc.d", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
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
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->name);
		$cmdArgs[] = $action;
		$cmd = new \OMV\System\Process("invoke-rc.d", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Invoke init script with parameter 'status'.
	 * @return One of the following exit status codes:
	 * 0        program is running or service is OK
	 * 1        program is dead and /run pid file exists
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
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->name);
		$cmdArgs[] = "status";
		$cmd = new \OMV\System\Process("invoke-rc.d", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		return $exitStatus;
	}
}
