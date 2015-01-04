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
require_once("openmediavault/log.inc");
require_once("openmediavault/functions.inc");

///////////////////////////////////////////////////////////////////////////////
// Register default logfile types.
///////////////////////////////////////////////////////////////////////////////

// /var/log/syslog:
// Oct 19 04:24:36 openmediavault init: Switching to runlevel: 6
// Oct 19 04:24:38 openmediavault watchdog[1589]: stopping daemon (5.9)
OMVLogFile::registerType("syslog", array(
  "filename" => "syslog",
  "filepath" => "/var/log/syslog",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"M j G:i:s");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"M j G:i:s"));')
	  ),
	  "hostname" => 2,
	  "message" => 3
  )));

// /var/log/daemon.log:
// Oct 19 04:25:07 openmediavault dhclient: Sending on   Socket/fallback
// Oct 19 04:25:08 openmediavault wd_keepalive[1164]: stopping watchdog keepalive daemon (5.9)
OMVLogFile::registerType("daemon", array(
  "filename" => "daemon.log",
  "filepath" => "/var/log/daemon.log",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"M j G:i:s");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"M j G:i:s"));')
	  ),
	  "hostname" => 2,
	  "message" => 3
  )));

// /var/log/auth.log:
// Oct 19 04:24:44 openmediavault sshd[1550]: Received signal 15; terminating.
// Oct 19 04:24:44 openmediavault sshd[2472]: Exiting on signal 15
OMVLogFile::registerType("auth", array(
  "filename" => "auth.log",
  "filepath" => "/var/log/auth.log",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"M j G:i:s");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"M j G:i:s"));')
	  ),
	  "hostname" => 2,
	  "message" => 3
  )));

// /var/log/messages:
// Oct 19 04:24:36 openmediavault shutdown[9419]: shutting down for system reboot
// Oct 19 04:24:49 openmediavault kernel: Kernel logging (proc) stopped.
OMVLogFile::registerType("messages", array(
  "filename" => "messages",
  "filepath" => "/var/log/messages",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"M j G:i:s");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"M j G:i:s"));')
	  ),
	  "hostname" => 2,
	  "message" => 3
  )));

// /var/log/boot:
// Fri Oct 19 04:25:05 2012: Setting parameters of disc: (none).
// Fri Oct 19 04:25:05 2012: Setting preliminary keymap...done.
OMVLogFile::registerType("boot", array(
  "filename" => "boot",
  "filepath" => "/var/log/boot",
  "regex" => "/^(\S+\s+\S+\s+\d+\s+\S+\s+\d+):\s+(.*)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"D M j H:i:s Y");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"D M j H:i:s Y"));')
	  ),
	  "message" => 2
  )));
?>
