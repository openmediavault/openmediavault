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
namespace OMV;

/**
 * \em ExecException is thrown when the execution of a process fails.
 * @ingroup api
 */
class ExecException extends BaseException {
	private $cmdLine = NULL;
	private $output = [];
	private $exitStatus = NULL;

	/**
	 * @param string $cmdLine The command line string or a
	 *   \OMV\System\Process instance.
	 * @param array $output The output of the executed command.
	 * @param int $exitStatus The return status of the executed command.
	 */
	public function __construct($cmdLine, array $output = array(),
			$exitStatus = NULL) {
	  	if ($cmdLine instanceof \OMV\System\Process) {
			$cmdLine->obfuscateEnvs();
	  	}
		$this->cmdLine = strval($cmdLine);
		$this->output = $output;
		$this->exitStatus = $exitStatus;
		$args = [];
		$args[] = $this->cmdLine;
		if (!is_null($this->exitStatus)) {
			$format = "Failed to execute command '%s' with exit code '%d': %s";
			$args[] = $this->exitStatus;
		} else {
			$format = "Failed to execute command '%s': %s";
		}
		$args[] = implode("\n", $output);
		parent::__construct(vsprintf($format, $args));
	}

	public function getCmdLine() {
		return $this->cmdLine;
	}

	public function getOutput() {
		return $this->output;
	}

	public function getExitStatus() {
		return $this->exitStatus;
	}
}
