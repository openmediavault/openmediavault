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
require_once("openmediavault/env.inc");

$prd = new \OMV\ProductInfo();
?>
<!DOCTYPE html>
<html>
	<head>
		<title><?=$prd->getName();?></title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<link rel="shortcut icon" type="image/x-icon" href="favicon.ico"/>
		<link rel="stylesheet" type="text/css" href="/extjs6/classic/theme-triton/resources/theme-triton-all.css"/>
		<link rel="stylesheet" type="text/css" href="css/theme-all.min.css"/>
	</head>
	<body class="shutdown-page">
		<span class="shutdown-icon x-fa fa-stack fa-2x">
			<i class="x-color-red x-fa fa fa-stack-2x fa-circle"></i>
			<i class="x-color-white x-fa fa fa-stack-1x fa-power-off"></i>
		</span>
		<a title='<?=$prd->getName();?>' href='<?=$prd->getURL();?>' target='_blank'><div class="product-logo"></div></a>
	</body>
</html>
