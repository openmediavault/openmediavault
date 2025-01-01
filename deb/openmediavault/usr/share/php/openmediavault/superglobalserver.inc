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

class SuperGlobalServer extends Dictionary {
	/**
	 * Constructor.
	 * @param array $data The data of the super global variable. Defaults
	 *   to `$_SERVER`.
	 * @see https://www.php.net/manual/en/reserved.variables.server.php
	 */
    public function __construct(array $data = NULL) {
    	parent::__construct($data ?? $_SERVER);
    }

	public function getUserAgent(): string {
        return $this->get("HTTP_USER_AGENT", "");
    }

    public function getServerAddr(): string {
        return $this->get("SERVER_ADDR", "");
    }

	/**
	 * Get the IP address from a client request by extracting it from a given
	 * list of headers. Those headers that are typically set by reverse proxies
	 * are also taken into account.
	 * @return Returns the IP address of the client.
	 */
    public function getClientIP(): string {
		$requestHeaders = [
			// https://developers.cloudflare.com/fundamentals/reference/http-request-headers/#cf-connecting-ip
			'HTTP_CF_CONNECTING_IP',
			'HTTP_CLIENT_IP',
			'HTTP_FORWARDED',
			// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/X-Forwarded-For
			'HTTP_FORWARDED_FOR',
			'HTTP_X_CLUSTER_CLIENT_IP',
			'HTTP_X_FORWARDED',
			'HTTP_X_FORWARDED_FOR',
			'HTTP_X_REAL_IP'
		];
		foreach ($requestHeaders as $headerk => $headerv) {
			if (NULL !== ($value = $this->get($headerv))) {
				$parts = explode(",", $value);
				$ip = trim($parts[0]);
				if (FALSE !== filter_var($ip, FILTER_VALIDATE_IP)) {
					return $ip;
				}
			}
		}
		return $this->get("REMOTE_ADDR", "");
	}
}
