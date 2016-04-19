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
namespace OMV\System;

/**
 * @ingroup api
 */
class Process {
	private $cmdArgs = array();
	private $redirect2to1 = FALSE;
	private $quiet = FALSE;
	private $background = FALSE;
	private $env = array(
		"PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin",
		"LANG=C"
	);

	/**
	 * Example:
	 * $cmd = new \OMV\System\Process("ls -alh");
	 * $cmd = new \OMV\System\Process("ls", "-al", "-h");
	 * $cmd = new \OMV\System\Process(array("ls", "-alh"));
	 * $cmd = new \OMV\System\Process("ls", array("-a", "-l", "-h"));
	 */
	public function __construct() {
		$args = func_get_args();
		if (1 == func_num_args() && is_string($args[0])) {
			$this->cmdArgs[] = $args[0];
		} else if (1 == func_num_args() && is_array($args[0])) {
			$this->cmdArgs = $args[0];
		} else if (2 == func_num_args() && is_string($args[0])) {
			$this->cmdArgs[] = $args[0];
			if (is_array($args[1]))
				$this->cmdArgs = array_merge($this->cmdArgs, $args[1]);
			else
				$this->cmdArgs[] = $args[1];
		} else {
			$this->cmdArgs = $args;
		}
	}

	public function setRedirect2to1($enable = TRUE) {
		$this->redirect2to1 = $enable;
	}

	public function setQuiet($quiet = TRUE) {
		$this->quiet = $quiet;
	}

	public function setBackground($background = TRUE) {
		$this->background = $background;
	}

	public function setEnv($string) {
		$this->env[] = $string;
	}

	/**
	 * Get the command line to be executed.
	 * @return The command line.
	 */
	public function getCommandLine() {
		$cmdArgs = array();
		foreach ($this->env as $env)
			$cmdArgs[] = sprintf("export %s;", $env);
		$cmdArgs = array_merge($cmdArgs, $this->cmdArgs);
		if (TRUE === $this->redirect2to1)
			$cmdArgs[] = "2>&1";
		if (TRUE === $this->background)
			$cmdArgs[] = "&";
		return implode(" ", $cmdArgs);
	}

	/**
	 * Execute the command.
	 * @param output If the output argument is present, then the specified
	 *   array will be filled with every line of output from the command.
	 * @param exitStatus If the argument is present along with the output
	 *   argument, then the return status of the executed command will be
	 *   written to this variable.
	 * @throw \OMV\ExecException
	 */
	public function execute(array &$output = NULL, &$exitStatus = NULL) {
		$cmdLine = $this->getCommandLine();
		$result = exec($cmdLine, $output, $exitStatus);
		if ((FALSE === $this->quiet) && (0 !== $exitStatus))
			throw new \OMV\ExecException($cmdLine, $output, $exitStatus);
		return $result;
	}
}
