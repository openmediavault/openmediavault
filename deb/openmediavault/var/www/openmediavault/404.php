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
try {
	require_once("openmediavault/autoloader.inc");

	http_response_code(404);
	$page = new \OMV\ControlPanel\Http404();
	$page->render();
} catch(\Exception $e) {
	// Send an error message to the web server's error log.
	error_log($e->getMessage());
	// Print the error message.
	header("Content-Type: text/html");
	http_response_code(($e instanceof \OMV\BaseException) ?
		$e->getHttpStatusCode() : 500);
	printf("Error #".$e->getCode().":<br/>%s", str_replace("\n", "<br/>",
		$e->__toString()));
}
?>
