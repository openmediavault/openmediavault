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
 * A specialized RPC proxy that handles file downloads.
 * @ingroup api
 */
class Download extends Json {
	/**
	 * @param response The response must contain the fields \em content or
	 *   \em filepath. The field \em filename and \em headers are optional.
	 *   Use Train-Case for the keys of the \em headers array.
	 *   The field \em content contains the content to be returned whereas
	 *   \em filepath contains the name of the file which content should be
	 *   returned. Please note that the file must be readable for everyone.
	 *   The fields \em content and \em filepath exclude each other.
	 *   If \em filename is set, the `Content-Disposition` header will be
	 *   set to `attachment; filename=<filename>`.
	 *   If \em unlink is set then the given file will be removed after use.
	 */
	protected function handleResponse($response) {
		// Check if the RPC response is plain text or complies the above
		// method parameter description. If it is plain text, then generate
		// the required data structure.
		if (!is_array($response)) {
			$response = [
				"filename" => sprintf("content%d", time()),
				"content" => $response
			];
		}
		// Set default values.
		$response = array_merge([
			"unlink" => FALSE
		], $response);
		// Prepare the headers.
		$headers = [];
		if (array_key_exists("content", $response)) {
			if (!(array_key_exists("headers", $response) && array_key_exists(
					"Content-Type", $response['headers']))) {
				$contentType = @mime_content_type($response['content']);
				if (FALSE !== $contentType) {
					$headers["Content-Type"] = $contentType;
				}
			}
			$headers["Content-Length"] = strlen($response['content']);
		} else {
			if (!(array_key_exists("headers", $response) && array_key_exists(
					"Content-Type", $response['headers']))) {
				$contentType = @mime_content_type($response['filepath']);
				if (FALSE !== $contentType) {
					$headers["Content-Type"] = $contentType;
				}
			}
			$contentLength = @filesize($response['filepath']);
			if (FALSE !== $contentLength) {
				$headers["Content-Length"] = $contentLength;
			}
		}
		if (array_key_exists("filename", $response)) {
			$headers["Content-Disposition"] = "attachment; filename=".
				$response['filename'];
		}
		if (array_key_exists("headers", $response)) {
			$headers = array_merge($headers, $response['headers']);
		}
		// Send the content to the client.
		foreach ($headers as $headerk => $headerv) {
			header("{$headerk}: {$headerv}");
		}
		if (array_key_exists("content", $response)) {
			print($response['content']);
		} else {
			@readfile($response['filepath']);
			// Unlink the file containing the content to be downloaded?
			if (TRUE === $response['unlink']) {
				@unlink($response['filepath']);
			}
		}
	}
}
