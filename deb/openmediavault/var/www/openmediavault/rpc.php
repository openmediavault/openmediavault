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

	// Load and initialize the RPC services that are not handled by the
	// engine daemon.
	$directory = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
		"OMV_DOCUMENTROOT_DIR"), "rpc");
	foreach (listdir($directory, "inc") as $path) {
		require_once $path;
	}
	$rpcServiceMngr = &\OMV\Rpc\ServiceManager::getInstance();
	$rpcServiceMngr->initializeServices();

	// Initialize the data models.
	$modelMngr = \OMV\DataModel\Manager::getInstance();
	$modelMngr->load();

	$session = &\OMV\Session::getInstance();
	$session->start();

	$server = new \OMV\Rpc\Proxy\Json();
	$server->handle();
} catch(\Exception $e) {
	header("Content-Type: application/json");
	http_response_code(($e instanceof \OMV\BaseException) ?
		$e->getHttpStatusCode() : 500);
	print json_encode_safe([
		"response" => null,
		"error" => [
			"code" => $e->getCode(),
			"message" => $e->getMessage(),
			"trace" => $e->__toString()
		]
	]);
} finally {
	if (isset($server)) {
		$server->cleanup();
	}
}
?>
