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
	private $command;
	private $redirect2to1 = FALSE;
	private $quiet = FALSE;
	private $background = FALSE;
	private $env = array(
		"PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin",
		"LANG=C"
	);

	public function __construct() {
		$args = func_get_args();
		if (1 == func_num_args() && is_string($args[0])) {
			$this->command = $args[0];
		} else if (1 == func_num_args() && is_array($args[0])) {
			$this->command = $args[0];
		} else if (2 == func_num_args() && is_string($args[0])) {
			$this->command = array();
			$this->command[] = $args[0];
			if (is_array($args[1]))
				$this->command = array_merge($this->command, $args[1]);
			else
				$this->command[] = $args[1];
		} else {
			$this->command = $args;
		}
		if (is_array($this->command))
			$this->command = implode(" ", $this->command);
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

	public function execute(&$output = NULL, &$exitStatus = NULL) {
		$parts = array();
		foreach ($this->env as $env)
			$parts[] = sprintf("export %s;", $env);
		$parts[] = $this->command;
		if (TRUE === $this->redirect2to1)
			$parts[] = "2>&1";
		if (TRUE === $this->background)
			$parts[] = "&";
		$command = implode(" ", $parts);
		$result = exec($command, $output, $exitStatus);
		if ((FALSE === $this->quiet) && ($exitStatus !== 0))
			throw new \OMV\ExecException($command, $output);
		return $result;
	}
}
