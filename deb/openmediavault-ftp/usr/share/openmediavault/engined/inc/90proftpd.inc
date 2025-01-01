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
require_once("openmediavault/functions.inc");

// Retrieved via journalctl.
// Oct 17 18:19:00 vmomv5 proftpd[17127]: 192.167.178.101 (::ffff:192.168.178.254[::ffff:192.168.178.254]) - USER test: Login successful.
// Oct 17 18:20:00 vmomv5 proftpd: pam_unix(proftpd:session): session closed for user test
\OMV\System\LogFileSpec::registerSpecification("proftpd", [
  "command" => "export SYSTEMD_COLORS=0; journalctl --quiet --no-pager ".
	"SYSLOG_IDENTIFIER='proftpd' --output=short",
  "filename" => "proftpd.log",
  "regex" => "/^(\S+\s+\d+\s+\d{2}:\d{2}:\d{2})\s+(\S+)\sproftpd\[\d+\]:\s+(.*)$/",
  "columns" => [
	  "ts" => [
		  "index" => 1,
		  "func" => function($v) { return strpdate($v,"M j G:i:s"); }
	  ],
	  "date" => [
		  "index" => 1,
		  "func" => function($v) {
			  return strftime("%c", strpdate($v,"M j G:i:s"));
		  }
	  ],
	  "hostname" => 2,
	  "message" => 3
  ]]);

// /var/log/proftpd/xferlog
// <currenttime> <transfertime> <remotehost> <filesize> <filename> <transfertype> <specialactionflag> <direction> <accessmode> <username> <servicename> <authenticationmethod> <authenticateduserid> <completionstatus>
// Thu Oct 17 18:19:00 2013 0 ::ffff:192.168.178.254 56374 /home/ftp/test/testfile b _ i r test ftp 0 * c
\OMV\System\LogFileSpec::registerSpecification("proftpd_xferlog", [
  "filename" => "xferlog",
  "filepath" => "/var/log/proftpd/xferlog",
  "regex" => "/^(\S+\s+\S+\s+\d+\s+\d{2}:\d{2}:\d{2}\s+\d+)\s+(\d+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(a|b)\s+(C|U|T|_)\s+(o|i|d)\s+(a|g|r)\s+(\S+)\s+(\S+)\s+(0|1)\s+(\d+|\*)\s+(c|i)$/",
  "columns" => [
	  "ts" => [
		  "index" => 1,
		  "func" => function($v) { return strpdate($v,"D M d H:i:s Y"); }
	  ],
	  "date" => [
		  "index" => 1,
		  "func" => function($v) {
			  return strftime("%c", strpdate($v,"D M d H:i:s Y"));
		  }
	  ],
	  "transfertime" => 2,
	  "remotehost" => 3,
	  "filesize" => 4,
	  "filename" => 5,
	  "transfertype" => 6,
	  "specialactionflag" => 7,
	  "direction" => 8,
	  "accessmode" => 9,
	  "username" => 10,
	  "servicename" => 11,
	  "authenticationmethod" => 12,
	  "authenticateduserid" => 13,
	  "completionstatus" => 14
  ]]);
