<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
	require_once("openmediavault/env.inc");
	require_once("openmediavault/config.inc"); // Must be included here
	require_once("openmediavault/session.inc");
	require_once("openmediavault/rpcproxy.inc");

	$session = &OMVSession::getInstance();
	$session->start();

	$server = new OMVJsonRpcProxy();
	$server->handle();
	$server->cleanup();
} catch(Exception $e) {
	if (isset($server))
		$server->cleanup();
	header("Content-Type: application/json");
	print json_encode_safe(array(
		"response" => null,
		"error" => array(
			"code" => $e->getCode(),
			"message" => $e->getMessage(),
			"trace" => $e->__toString()
		)
	));
}
?>
