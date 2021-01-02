<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/env.inc");
require_once("openmediavault/functions.inc");

if (array_keys_exists(array("service", "method"), $_POST)) {
	try {
		function exception_error_handler($errno, $errstr, $errfile,
		  $errline) {
			switch ($errno) {
			case E_STRICT:
				break;
			default:
				throw new \ErrorException($errstr, 0, $errno, $errfile,
				  $errline);
				break;
			}
		}
		set_error_handler("exception_error_handler");

		$session = &\OMV\Session::getInstance();
		$session->start();
		$session->validate();
		// Do not update last access time.
		//$session->updateLastAccess();

		$server = new \OMV\Rpc\Proxy\Download();
		$server->handle();
		$server->cleanup();
	} catch(\Exception $e) {
		if (isset($server))
			$server->cleanup();
		header("Content-Type: text/html");
		http_response_code(($e instanceof \OMV\BaseException) ?
			$e->getHttpStatusCode() : 500);
		printf("Error #%s:<br/>%s", strval($e->getCode()),
			str_replace("\n", "<br/>", htmlentities($e->__toString())));
	}
} else {
	// Return the HTML code of the form containing the fields required
	// to process the download request.
	print <<<EOF
<html>
	<head></head>
	<form method="post">
		<input type="hidden" name="service" value=""/>
		<input type="hidden" name="method" value=""/>
		<input type="hidden" name="params" value=""/>
	</form>
</html>
EOF;
}
?>
