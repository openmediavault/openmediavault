<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
namespace OMV\Rpc;

/**
 * \em TraceException is a special exception that should be used when a
 * stack trace should be delivered to the exception catcher.
 * @ingroup api
 */
class TraceException extends Exception {
	private $trace; /**< The exception stack trace. */

	/**
	 * Constructor
	 * @param message The exception message to throw.
	 * @param code The exception code.
	 * @param trace The exception stack trace.
	 */
	public function __construct($message, int $code = 0, string $trace = "") {
		$this->trace = $trace;
		parent::__construct($message, $code);
	}

	/**
	 * String representation of the exception.
	 * @return Returns the string representation of the exception.
	 */
	public function __toString() : string {
		return $this->trace;
	}
}
