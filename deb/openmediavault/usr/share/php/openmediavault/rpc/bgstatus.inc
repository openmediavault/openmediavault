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
namespace OMV\Rpc;

/**
 * Helper class to handle the background process status.
 * @ingroup api
 */
class BgStatus {
	private $status = [];

	/**
	 * Constructor
	 * @param array status An array of status informations.
	 */
	public function __construct(array $status) {
		$this->status = $status;
	}

	/**
	 * Get the PID of the background process.
	 * @return int The PID of the background process.
	 */
	public function getPid(): int {
		return $this->get("pid");
	}

	/**
	 * Check whether the background process is running.
	 * @return bool Returns TRUE if the background process is runnig,
	 *   otherwise FALSE.
	 */
	public function isRunning(): bool {
		return $this->get("running");
	}

	/**
	 * Check whether the background process is has thrown an error.
	 * @return bool Returns TRUE if the background process has been thrown
	 *   an error, otherwise FALSE.
	 */
	public function hasError(): bool {
		return array_key_exists("error", $this->status) && !is_null(
		  $this->status['error']);
	}

	/**
	 * Get the error object.
	 * @return array The error object containing the fields \em code,
	 *   \em message and \em trace.
	 */
	public function getError(): array {
		return $this->get("error");
	}

	/**
	 * Check whether the background process output is written to a temporary
	 * file.
	 * @return bool Returns TRUE if the background process writes its
	 *   output to a temporary file, otherwise FALSE.
	 */
	public function hasOutputFilename(): bool {
		return array_key_exists("outputfilename", $this->status);
	}

	/**
	 * Get the name of the temporary file that contains the output of the
	 * background job.
	 * @return string Returns the name of the temporary file.
	 */
	public function getOutputFilename(): string {
		return $this->get("outputfilename");
	}

	/**
	 * Helper method to access the status fields.
	 * @param string key The name of the status field.
	 * @return mixed The value of the given status field.
	 */
	public function get(string $key) {
		return $this->status[$key];
	}

	/**
	 * Helper method to set the status fields.
	 * @param string key The name of the status field.
	 * @param mixed value The value of the status field.
	 * @return mixed The value of the given status field.
	 */
	public function set(string $key, $value) {
		$this->status[$key] = $value;
		return $this->get($key);
	}
}
