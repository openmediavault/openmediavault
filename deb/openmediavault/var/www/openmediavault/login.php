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
try {
	require_once("openmediavault/autoloader.inc");
	require_once("openmediavault/env.inc");
	require_once("openmediavault/functions.inc");

	if (!(isset($_SERVER['PHP_AUTH_USER']) && isset($_SERVER['PHP_AUTH_PW']))) {
		header("WWW-Authenticate: Basic");
		http_response_code(401);
		die("Authentication failed");
	}

	// Load and initialize the RPC services that are not handled by the
	// engine daemon.
	$directory = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
		"OMV_DOCUMENTROOT_DIR"), "rpc");
	foreach (listdir($directory, "inc") as $path) {
		require_once $path;
	}
	$rpcServiceMngr = &\OMV\Rpc\ServiceManager::getInstance();
	$rpcServiceMngr->initializeServices();

	$session = &\OMV\Session::getInstance();
	$session->start();

	$server = new \OMV\Rpc\Proxy\Login();
	$server->handle();
} catch(\Exception $e) {
	http_response_code(($e instanceof \OMV\BaseException) ?
		$e->getHttpStatusCode() : 500);
	die($e->getMessage());
} finally {
	if (isset($server)) {
		$server->cleanup();
	}
}
?>
