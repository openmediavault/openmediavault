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
try {
	require_once("openmediavault/autoloader.inc");
	require_once("openmediavault/functions.inc");

	$session = &\OMV\Session::getInstance();
	$session->start();
	$session->validate();
	// Do not update last access time.
	//$session->updateLastAccess();

	// The parameter 'name' may not contain the characters '..'. This is
	// because of security reasons: the given canonicalized absolute
	// path MUST be below the given image directory.
	if (1 == preg_match("/\.\./", $_GET['name'])) {
		throw new \OMV\Exception(
		  "The parameter 'name' contains forbidden two-dot symbols.");
	}
	// Build the image file path. If it does not exist, then display an
	// error image by default.
	$pathName = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
		"OMV_RRDGRAPH_DIR"), $_GET['name']);
	if (!file_exists($pathName))
		$pathName = \OMV\Environment::get("OMV_RRDGRAPH_ERROR_IMAGE");
	$fd = fopen($pathName, "r");
	header("Content-type: image/png");
	fpassthru($fd);
} catch(\Exception $e) {
	header("Content-Type: text/html");
	http_response_code(($e instanceof \OMV\BaseException) ?
		$e->getHttpStatusCode() : 500);
	printf("Error #%s:<br/>%s", strval($e->getCode()),
		str_replace("\n", "<br/>", htmlentities($e->__toString())));
}
?>
