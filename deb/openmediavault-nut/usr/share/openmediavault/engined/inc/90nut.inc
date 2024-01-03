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
require_once("openmediavault/functions.inc");

// Retrieved via journalctl.
// Feb 20 10:53:49 omv6box upsd[14278]: upsd disabled, please adjust the configuration to your needs
// Feb 20 10:53:49 omv6box upsd[14278]: Then set MODE to a suitable value in /etc/nut/nut.conf to enable it
// Feb 20 10:53:51 omv6box upsd[14398]: listening on ::1 port 3493
// Feb 20 10:53:51 omv6box upsd[14398]: listening on 127.0.0.1 port 3493
// Feb 20 10:53:51 omv6box upsd[14398]: fopen /run/nut/upsd.pid: No such file or directory
\OMV\System\LogFileSpec::registerSpecification("nut", [
  "command" => "export SYSTEMD_COLORS=0; journalctl --quiet --no-pager ".
  	"SYSLOG_IDENTIFIER='upsd' + SYSLOG_IDENTIFIER='upsmon' + ".
  	"SYSLOG_IDENTIFIER='upssched' + SYSLOG_IDENTIFIER='upssched-cmd' ".
  	"--output=short",
  "filename" => "nut.log",
  "regex" => "/^(\S+\s+\d+\s+\d{2}:\d{2}:\d{2})\s+(\S+)\s([a-zA-Z-]+)(\[(\d+)\])?:\s+(.*)$/",
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
	  "program" => 3,
	  "pid" => 5,
	  "message" => 6
  ]]);
