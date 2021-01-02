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
require_once("openmediavault/functions.inc");

// /var/log/syslog:
// Oct 19 04:24:36 openmediavault init: Switching to runlevel: 6
// Oct 19 04:24:38 openmediavault watchdog[1589]: stopping daemon (5.9)
\OMV\System\LogFileSpec::registerSpecification("syslog", [
  "filename" => "syslog",
  "filepath" => "/var/log/syslog",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
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

// /var/log/daemon.log:
// Oct 19 04:25:07 openmediavault dhclient: Sending on   Socket/fallback
// Oct 19 04:25:08 openmediavault wd_keepalive[1164]: stopping watchdog keepalive daemon (5.9)
\OMV\System\LogFileSpec::registerSpecification("daemon", [
  "filename" => "daemon.log",
  "filepath" => "/var/log/daemon.log",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
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

// /var/log/auth.log:
// Oct 19 04:24:44 openmediavault sshd[1550]: Received signal 15; terminating.
// Oct 19 04:24:44 openmediavault sshd[2472]: Exiting on signal 15
\OMV\System\LogFileSpec::registerSpecification("auth", [
  "filename" => "auth.log",
  "filepath" => "/var/log/auth.log",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
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

// /var/log/messages:
// Oct 19 04:24:36 openmediavault shutdown[9419]: shutting down for system reboot
// Oct 19 04:24:49 openmediavault kernel: Kernel logging (proc) stopped.
\OMV\System\LogFileSpec::registerSpecification("messages", [
  "filename" => "messages",
  "filepath" => "/var/log/messages",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
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

// Retrieved via journalctl.
// 2016-06-26T21:24:08+0200 vmpc05 kernel: Booting paravirtualized kernel on bare hardware
// 2016-06-26T21:24:08+0200 vmpc05 kernel: SCSI subsystem initialized
\OMV\System\LogFileSpec::registerSpecification("boot", [
  "command" => "export SYSTEMD_COLORS=0; journalctl --boot=-0 --quiet ".
    "--output=short-iso --no-pager",
  "filename" => "boot",
  "regex" => "/^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}([+-]\d{4})?)\s+(\S+)\s+(.+)$/",
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
	  "hostname" => 3,
	  "message" => 4
  ]]);

// /var/log/dpkg.log
// 2016-02-11 11:47:02 status unpacked openmediavault-clamav:all 3.0.2
// 2016-02-11 11:47:02 status half-configured openmediavault-clamav:all 3.0.2
// 2016-02-12 15:06:26 status unpacked php5-common:amd64 5.6.14+dfsg-0+deb8u1
// 2016-02-12 15:06:26 status half-installed php5-common:amd64 5.6.14+dfsg-0+deb8u1
\OMV\System\LogFileSpec::registerSpecification("dpkg", [
  "filename" => "dpkg.log",
  "filepath" => "/var/log/dpkg.log",
  "regex" => "/^(\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2})\s+(.+)$/",
  "columns" => [
	  "ts" => [
		  "index" => 1,
		  "func" => function($v) { return strpdate($v,"Y-m-d H:i:s"); }
	  ],
	  "date" => [
		  "index" => 1,
		  "func" => function($v) {
			  return strftime("%c", strpdate($v,"Y-m-d H:i:s"));
		  }
	  ],
	  "message" => 2
  ]]);

// Retrieved via journalctl.
// Oct  5 08:58:35 vmomv5 rsyncd[2645]: rsyncd version 3.1.1 starting, listening on port 873
\OMV\System\LogFileSpec::registerSpecification("rsyncd", [
  "command" => "export SYSTEMD_COLORS=0; journalctl --quiet --no-pager ".
    "SYSLOG_IDENTIFIER='rsyncd' --output=short",
  "filename" => "rsyncd",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+rsyncd\[\d+\]:\s+(.*)$/",
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

// Retrieved via journalctl.
// Oct  5 08:48:09 vmomv5 smartd[2094]: smartd 6.4 2014-10-07 r4002 [x86_64-linux-3.16.0-4-amd64] (local build)
// Oct  5 08:48:09 vmomv5 smartd[2094]: Copyright (C) 2002-14, Bruce Allen, Christian Franke, www.smartmontools.org
// Oct  5 08:48:09 vmomv5 smartd[2094]: Opened configuration file /etc/smartd.conf
\OMV\System\LogFileSpec::registerSpecification("smartd", [
  "command" => "export SYSTEMD_COLORS=0; journalctl --quiet --no-pager ".
    "SYSLOG_IDENTIFIER='smartd' --output=short",
  "filename" => "smartd.log",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\ssmartd\[\d+\]:\s+(.*)$/",
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
	  "message" => 3
  ]]);

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
  "regex" => "/^(\S+\s+\S+\s+\d+\s+\d{2}:\d{2}:\d{2}\s+\d+)\s+(\d+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(a|b)\s+(C|U|T|_)\s+(o|i)\s+(a|g|r)\s+(\S+)\s+(\S+)\s+(0|1)\s+(\d+|\*)\s+(c|i)$/",
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

// Retrieved via journalctl.
// Mar 13 23:50:48 vmomv5 smbd_audit[pid]: <username>|<ipaddr>|<clientnetbiosname>|<servicerootdir>|<servicename>|<operation>|<result>|<filename>
// Mar 13 23:50:48 vmomv5 smbd_audit[4192]: test|192.168.178.30|pc01|/media/1622d31c-7f5d-4add-be13-20e382a9f5cb/test|download|pwrite|ok|xyz.tar.gz
\OMV\System\LogFileSpec::registerSpecification("smbdaudit", [
  "command" => "export SYSTEMD_COLORS=0; journalctl --quiet --no-pager ".
    "--priority='notice' --identifier='smbd_audit' --output=short",
  "filename" => "smbd_audit.log",
  "regex" => "/^(\S+\s+\d+\s+\d{2}:\d{2}:\d{2})\s+(\S+)\ssmbd_audit\[\d+\]:\s+([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)$/",
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
	  "username" => 3,
	  "clientipaddr" => 4,
	  "clientnetbiosname" => 5,
	  "servicerootdir" => 6,
	  "servicename" => 7,
	  "operation" => 8,
	  "result" => 9,
	  "filepath" => 10,
	  "filename" => [
	      "index" => 10,
	      "func" => function($v) { return basename($v); }
	  ]
  ]]);

// /var/log/rsync.log
// 2016/10/05 14:19:36 [3586] building file list
// 2016/10/05 14:19:36 [3586] .d..t...... ./
// 2016/10/05 14:19:36 [3586] >f+++++++++ COPYING
\OMV\System\LogFileSpec::registerSpecification("rsync", [
  "filename" => "rsync",
  "filepath" => "/var/log/rsync.log",
  "regex" => "/^(\S+\s+\S+) \[\d+\] (.*)$/",
  "columns" => [
	  "ts" => [
		  "index" => 1,
		  "func" => function($v) { return strpdate($v,"Y/m/d G:i:s"); }
	  ],
	  "date" => [
		  "index" => 1,
		  "func" => function($v) {
			  return strftime("%c", strpdate($v,"Y/m/d G:i:s"));
		  }
	  ],
	  "message" => 2
  ]]);

// /var/log/apt/term.log
// Preparing to unpack .../base-files_8+deb8u3_amd64.deb ...
// Unpacking base-files (8+deb8u3) over (8+deb8u2) ...
// Processing triggers for install-info (5.2.0.dfsg.1-6) ...
// Processing triggers for man-db (2.7.0.2-5) ...
// Setting up base-files (8+deb8u3) ...
// Installing new version of config file /etc/debian_version ...
\OMV\System\LogFileSpec::registerSpecification("apt_term", [
  "filename" => "term.log",
  "filepath" => "/var/log/apt/term.log",
  "regex" => "/^(.+)$/",
  "columns" => [
	  "message" => 1
  ]]);

// /var/log/apt/history.log
// Start-Date: 2016-02-12  15:05:18
// Commandline: apt-get --yes --force-yes --fix-missing --auto-remove ...
// Upgrade: bind9-host:amd64 (9.9.5.dfsg-9+deb8u4, 9.9.5.dfsg-9+deb8u5), ...
// End-Date: 2016-02-12  15:06:53
\OMV\System\LogFileSpec::registerSpecification("apt_history", [
  "filename" => "history.log",
  "filepath" => "/var/log/apt/history.log",
  "regex" => "/^(.+)$/",
  "columns" => [
	  "message" => 1
  ]]);
