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
class ShellScript {
	private $script = array();
	private $quiet = FALSE;

	public function __construct($script) {
		$this->script = $script;
	}

	public function setQuiet($quiet = TRUE) {
		$this->quiet = $quiet;
	}

	public function execute(&$output = NULL, &$exitStatus = NULL) {
		$cmdLine = $this->script;
		$result = exec($cmdLine, $output, $exitStatus);
		if ((FALSE === $this->quiet) && (0 !== $exitStatus))
			throw new \OMV\ExecException($cmdLine, $output);
		return $result;
	}
}
