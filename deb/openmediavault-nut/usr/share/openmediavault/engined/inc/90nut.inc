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
// 2024-08-23T19:12:15+0000 omv7box upsd[4934]: listening on ::1 port 3493
// 2024-08-23T19:12:15+0000 omv7box upsd[4934]: listening on 127.0.0.1 port 3493
\OMV\System\LogFileSpec::registerSpecification("nut", [
  "command" => "export SYSTEMD_COLORS=0; journalctl --quiet --no-pager ".
  	"SYSLOG_IDENTIFIER='upsd' + SYSLOG_IDENTIFIER='upsmon' + ".
  	"SYSLOG_IDENTIFIER='upssched' + SYSLOG_IDENTIFIER='upssched-cmd' ".
  	"--output=short-iso",
  "filename" => "nut.log",
  "regex" => "/^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}[+-]\d{4})\s+(\S+)\s([a-zA-Z-]+)(\[(\d+)\])?:\s+(.*)$/",
  "columns" => [
	  "ts" => [
		  "index" => 1,
		  "func" => function($v) { return strpdate($v,"Y-m-d*H:i:sT"); }
	  ],
	  "date" => [
		  "index" => 1,
		  "func" => function($v) {
			  return strftime("%c", strpdate($v,"Y-m-d*H:i:sT"));
		  }
	  ],
	  "hostname" => 2,
	  "program" => 3,
	  "pid" => 5,
	  "message" => 6
  ]]);
