<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2018 Volker Theile
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
class BgStatusFile {
	private $status = [];

	/**
	 * Constructor
	 * @param status An array of status informations.
	 */
	public function __construct($status) {
		$this->status = $status;
	}

	/**
	 * Get the PID of the background process.
	 * @return The PID of the background process.
	 */
	public function getPid() {
		return $this->get("pid");
	}

	/**
	 * Check whether the background process is running.
	 * @return Returns TRUE if the background process is runnig, otherwise
	 *   FALSE.
	 */
	public function isRunning() {
		return $this->get("running");
	}

	/**
	 * Check whether the background process is has thrown an error.
	 * @return Returns TRUE if the background process has been thrown an
	 *   error, otherwise FALSE.
	 */
	public function hasError() {
		return array_key_exists("error", $this->status) && !is_null(
		  $this->status['error']);
	}

	/**
	 * Get the error object.
	 * @return The error object containing the fields \em code, \em message
	 *   and \em trace.
	 */
	public function getError() {
		return $this->get("error");
	}

	/**
	 * Check whether the background process output is written to a temporary
	 * file.
	 * @return Returns TRUE if the background process writes his output to a
	 *   temporary file, otherwise FALSE.
	 */
	public function hasOutputFilename() {
		return array_key_exists("outputfilename", $this->status);
	}

	/**
	 * Get the name of the temporary file that contains the output of the
	 * background job.
	 * @return Returns the name of the temporary file.
	 */
	public function getOutputFilename() {
		return $this->get("outputfilename");
	}

	/**
	 * Helper method to access the status fields.
	 * @param key The name of the status field.
	 * @return The value of the given status field.
	 */
	public function get($key) {
		return $this->status[$key];
	}

	/**
	 * Helper method to set the status fields.
	 * @param key The name of the status field.
	 * @param value The value of the status field.
	 * @return The value of the given status field.
	 */
	public function set($key, $value) {
		$this->status[$key] = $value;
		return $this->get($key);
	}
}
