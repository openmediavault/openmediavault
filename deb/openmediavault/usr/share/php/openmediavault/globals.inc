<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
define("_PERMISSION_NA",  0);
define("_PERMISSION_X",   1);
define("_PERMISSION_W",   2);
define("_PERMISSION_WX",  3);
define("_PERMISSION_R",   4);
define("_PERMISSION_RX",  5);
define("_PERMISSION_RW",  6);
define("_PERMISSION_RWX", 7);

define("OMV_NOTIFY_PRECREATE", 0x1);
define("OMV_NOTIFY_CREATE",    0x2);
define("OMV_NOTIFY_PREMODIFY", 0x4);
define("OMV_NOTIFY_MODIFY",    0x8);
define("OMV_NOTIFY_PREDELETE", 0x10);
define("OMV_NOTIFY_DELETE",    0x20);

define("OMV_ROLE_ADMINISTRATOR", 0);
define("OMV_ROLE_USER",          1);

$OMV_DEFAULT_FILE = "/etc/default/openmediavault";
$OMV_RRDGRAPH_DIR = "/var/lib/openmediavault/rrd";
$OMV_DEBUG_PHP_LOGFILE = "/tmp/openmediavault_debug_php.log";
$OMV_UUID_UNDEFINED = "undefined";
$OMV_MAX_LOGFILE_LINES = "100000";
$OMV_HTTPREQUEST_TIMEOUT = "60000";

/**
 * Use the following table as reference when defining a regex:
 *  " => \"
 * \w => \\\w
 * \d => \\\d
 *  / => \\\/
 *  \ => \\\\
 *  [ => \\\[
 *  ] => \\\]
 *  $ => \\\?
 *  + => \\\+
 *  * => \\\*
 *  . => \\\.
 *  | => \\\|
 *  ? => \\\?
 *  : => \\\:
 */
$OMV_JSONSCHEMA_USERNAME = '"type":"string","format":"regex","pattern":'.
  '"\/^[_.A-Za-z0-9][-\\\@_.A-Za-z0-9]*\\\$?$\/i"';
$OMV_JSONSCHEMA_COUNTFIELD = '"type":["integer","null"]';
$OMV_JSONSCHEMA_SORTFIELD = '"type":["string","null"]';
$OMV_JSONSCHEMA_SORTDIR ='"type":["string","null"],"format":"regex",'.
  '"pattern":"\/^asc|desc$\/i"';
$OMV_JSONSCHEMA_UUID = '"type":"string","format":"regex","pattern":'.
  '"\/^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}$\/i"';
$OMV_JSONSCHEMA_UUID_UNDEFINED = '"type":"string","format":"regex",'.
  '"pattern":"\/^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}|'.
  $OMV_UUID_UNDEFINED.'$\/i"';
$OMV_JSONSCHEMA_UUID_NONE = '"type":"string","format":"regex","pattern":'.
  '"\/^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}|$\/i"';
$OMV_JSONSCHEMA_UUID_LIST = '"type":"string","format":"regex","pattern":'.
  '"\/^([a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}[,;])*'.
  '[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}$\/i"';
$OMV_JSONSCHEMA_DEVICEFILE = '"type":"string","format":"regex",'.
  '"pattern":"\/^\\\/dev(\\\/disk\\\/by-id)?\\\/.+$\/i"';
$OMV_JSONSCHEMA_DEVICEFILE_LIST = '"type":"string","format":"regex",'.
  '"pattern":"\/^(.+[,;])*.+$\/i"';
$OMV_JSONSCHEMA_EMAIL = '"type":"string","format":"regex","pattern":'.
  '"\/^(\\\w+)([-+.\'][\\\w]+)*@(\\\w[-\\\w]*\\\.){1,5}([A-Za-z]){2,6}|$\/"';
// JSON schema describing a file system UUID.
// EXT(2|3|4)/JFS/XFS: 7725c816-00d8-11e1-ad4c-00221568ca88
// DOS: 7A48-BA97
// NTFS: 2ED43920D438EC29
$OMV_JSONSCHEMA_FILESYSTEM_UUID = '"type":"string","format":"regex",'.
  '"pattern":"\/^([a-f0-9]{4}-[a-f0-9]{4}|[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-'.
  '[a-f0-9]{4}-[a-f0-9]{12}|[a-f0-9]{16})$\/i"';
// JSON schema describing a file system identifier. This can be a UUID or
// the devicefile of the file system.
$OMV_JSONSCHEMA_FILESYSTEM_ID = '"type":"string","format":"regex",'.
  '"pattern":"\/^([a-f0-9]{4}-[a-f0-9]{4}|[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-'.
  '[a-f0-9]{4}-[a-f0-9]{12}|[a-f0-9]{16}|\\\/dev\\\/.+)$\/i"';
$OMV_JSONSCHEMA_SHARENAME ='"type":["string","null"],"format":"regex",'.
  '"pattern":"\/^[^\\\.]([^\"\\\/\\\\\\\[\\\]\\\:\\\+\\\|\\\<\\\>=;,\\\*'.
  '\\\?\\\. ]+){0,1}([.][^\"\\\/\\\\\\\[\\\]\\\:\\\+\\\|\\\<\\\>=;,\\\*'.
  '\\\?\\\. ]+){0,}$\/i"';
$OMV_JSONSCHEMA_IPV4 = '"type":"string","format":"regex","pattern":'.
  '"\/^([1-9][0-9]{0,1}|1[013-9][0-9]|12[0-689]|2[01][0-9]|22[0-3])'.
  '([.]([1-9]{0,1}[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])){2}[.]'.
  '([1-9][0-9]{0,1}|1[0-9]{2}|2[0-4][0-9]|25[0-4])$\/i"';
$OMV_JSONSCHEMA_IPV4_NONE = '"type":"string","format":"regex","pattern":'.
  '"\/^([1-9][0-9]{0,1}|1[013-9][0-9]|12[0-689]|2[01][0-9]|22[0-3])'.
  '([.]([1-9]{0,1}[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])){2}[.]'.
  '([1-9][0-9]{0,1}|1[0-9]{2}|2[0-4][0-9]|25[0-4])|$\/i"';
$OMV_REGEX_DEVNAME_HARDDISK = "(h|s|x?v)d[a-z]+";
$OMV_REGEX_DEVNAME_SOFTWARERAID = "md\d+";
$OMV_REGEX_DEVNAME_HARDWARERAID = "cciss\/c[0-9]+d[0-9]+";

// Read the defaults file
$lines = file($GLOBALS['OMV_DEFAULT_FILE']);
foreach ($lines as $kline => $vline) {
	if (1 == preg_match("/^(OMV_[A-Z0-9_]+)=(\")?([^\"]+)(\")?$/",
	  trim($vline), $matches)) {
		$GLOBALS[$matches[1]] = $matches[3];
	}
	unset($matches);
}
unset($lines);
?>
