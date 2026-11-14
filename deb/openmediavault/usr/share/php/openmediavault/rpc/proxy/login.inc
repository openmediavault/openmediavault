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
 * A specialized RPC proxy that handles logins.
 * @ingroup api
 */
class Login extends Json {
	protected function getParams() {
		$this->params = [
			"service" => "Session",
			"method" => "login",
			"params" => [
				"username" => $_SERVER['PHP_AUTH_USER'],
				"password" => $_SERVER['PHP_AUTH_PW']
			]
		];
	}

	protected function handleResponse($response) {
		$username = $response['username'];
		$permissions = json_encode($response['permissions']);
		header("Content-Type: text/html; charset=utf-8");
		header("Cache-Control: max-age=0, no-cache, no-store, must-revalidate");
		header("Pragma: no-cache");
		header("Expires: 0");
		print <<<EOF
<!DOCTYPE html>
<html>
	<head>
		<script type="text/javascript">
			sessionStorage.setItem("username", "{$username}");
			sessionStorage.setItem("permissions", '{$permissions}');
			window.location.href = "/";
		</script>
	</head>
	<body></body>
</html>
EOF;
	}
}
