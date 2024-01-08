<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2024 Volker Theile
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
 * A specialized RPC proxy that handles file downloads.
 * @ingroup api
 */
class Download extends Json {
	protected function getParams() {
		$this->params = $_POST;
		// Decode the `params` field.
		$params = array_value($this->params, "params");
		if (is_string($params) && !empty($params)) {
		    $params = htmlspecialchars_decode($params);
			$this->params['params'] = json_decode($params, TRUE);
		}
		return TRUE;
	}

	/**
	 * @param response The response must contain the following fields
	 *   \em filename, \em content or \em filepath and \em contenttype which
	 *   is optional. The field \em content contains the content to be
	 *   returned, \em filepath contains the name of the file which content
	 *   should be returned. Please note that the file must be readable for
	 *   everybody. If \em unlink is set then the given file will be
	 *   removed after use. The fields \em content and \em filepath exclude
	 *   each other. Only The field \em contenttype contains the
	 *   'Content-Type' header string if set, otherwise it is set to
	 *   'text/plain'.
	 */
	protected function handleResponse($response) {
		// Check if the RPC response is plain text or complies the above
		// method parameter description. If it is plain text, then generate
		// the required data structure.
		if (!is_array($response)) {
			$response = array(
				"filename" => sprintf("content%d", time()),
				"content" => $response
			);
		}
		header("Content-Type: " . (array_key_exists("contenttype", $response) ?
		  $response['contenttype'] : "text/plain"));
		header('Expires: 0');
		header("Content-Disposition: attachment; " .
		  "filename={$response['filename']}");
		if (array_key_exists("content", $response)) {
			$contentLength = strlen($response['content']);
			header("Content-Length: {$contentLength}");
			print($response['content']);
		} else {
			@readfile($response['filepath']);
			// Unlink the file containing the content to be downloaded?
			if (array_key_exists("unlink", $response) &&
			  (TRUE === $response['unlink'])) {
				@unlink($response['filepath']);
			}
		}
	}
}
