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
 * @ingroup api
 */
class BaseException extends \Exception {
	/**
	 * @var int The HTTP status code that is used when the exception
	 *   is thrown while processing a RPC request. Defaults to 500.
	 */
	public $httpStatusCode = 500;

	/**
	 * Get the HTTP status code of this exception.
	 * @return int The HTTP status code.
	 */
	public function getHttpStatusCode() : int {
		return $this->httpStatusCode;
	}

	/**
	 * Set the HTTP status code of this exception.
	 * @param int $code The HTTP status code.
	 * @see https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
	 */
	public function setHttpStatusCode(int $code) {
		$this->httpStatusCode = $code;
	}
}
