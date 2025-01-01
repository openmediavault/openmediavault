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
namespace OMV\Rpc\Proxy;

/**
 * A specialized RPC proxy that handles file uploads.
 * @ingroup api
 */
class Upload extends Json {
	/**
	 * Copy the uploaded file to a temporary location. Modify this
	 * filename to allow the RPC service method to process the correct
	 * file.
	 */
	protected function preExecute() {
		// Is file successfully upload?
		if (!is_uploaded_file($_FILES['file']['tmp_name']))
			throw new Exception("Invalid upload request.");
		// Create the temporary filename.
		$tmpFilename = tempnam(sys_get_temp_dir(), "upload");
		// Copy file to a temporary location.
		if (!move_uploaded_file($_FILES['file']['tmp_name'], $tmpFilename)) {
			throw new Exception(
			  "Failed to move uploaded file (src=%s, dst=%s, size=%d).",
			  $_FILES['file']['tmp_name'], $tmpFilename,
			  $_FILES['file']['size']);
		}
		// Modify the service method parameters.
		$this->params['params']['filename'] = $_FILES['file']['name'];
		$this->params['params']['filepath'] = $tmpFilename;
	}

	/**
	 * ExtJS wants to see the response in a special format.
	 */
	protected function handleResponse($response) {
		header("Content-Type: text/html");
		print json_encode_safe([
			"success" => true, // required by ExtJS
			"responseText" => $response // required by ExtJS
		]);
	}

	/**
	 * The function that is called after the RPC has been successfully
	 * executed or it has been failed.
	 */
	public function cleanup() {
		// Ensure the uploaded file is removed.
		if (array_key_exists("filepath", $this->params['params'])) {
			if (file_exists($this->params['params']['filepath']))
				unlink($this->params['params']['filepath']);
		}
	}
}
