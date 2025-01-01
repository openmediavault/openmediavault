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
require_once("openmediavault/autoloader.inc");

/**
 * @defgroup api The Application Programming Interface
 */
define("OMV_NOTIFY_PRECREATE", 0x1);
define("OMV_NOTIFY_CREATE",    0x2);
define("OMV_NOTIFY_PREMODIFY", 0x4);
define("OMV_NOTIFY_MODIFY",    0x8);
define("OMV_NOTIFY_PREDELETE", 0x10);
define("OMV_NOTIFY_DELETE",    0x20);
define("OMV_NOTIFY_EVENT",     0x40);

define("OMV_ROLE_ADMINISTRATOR", 0x1);
define("OMV_ROLE_USER",          0x2);
define("OMV_ROLE_EVERYONE",      OMV_ROLE_ADMINISTRATOR | OMV_ROLE_USER);

define("OMV_STORAGE_DEVICE_TYPE_DISK", 0x1);
define("OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID", 0x2);
define("OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE", 0x4);
define("OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER", 0x8);
define("OMV_STORAGE_DEVICE_TYPE_SCSIGENERIC", 0x10); // !!! Character device
define("OMV_STORAGE_DEVICE_TYPE_ALL",
	OMV_STORAGE_DEVICE_TYPE_DISK |
	OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID |
	OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE |
	OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER);

define("OMV_NETWORK_INTERFACE_TYPE_ETHERNET", 0x1);
define("OMV_NETWORK_INTERFACE_TYPE_BOND", 0x2);
define("OMV_NETWORK_INTERFACE_TYPE_VLAN", 0x4);
define("OMV_NETWORK_INTERFACE_TYPE_BRIDGE", 0x8);
define("OMV_NETWORK_INTERFACE_TYPE_WIFI", 0x10);
define("OMV_NETWORK_INTERFACE_TYPE_VIRTUAL", 0x20);
define("OMV_NETWORK_INTERFACE_TYPE_LOOPBACK", 0x40);
define("OMV_NETWORK_INTERFACE_TYPE_ALL", 0xFF);

// Globals that can be modified via /etc/default/openmediavault.
\OMV\Environment::set("OMV_DEFAULT_FILE", "/etc/default/openmediavault");
\OMV\Environment::set("OMV_DEBUG_PHP_LOGFILE", "/tmp/openmediavault_debug_php.log");
\OMV\Environment::set("OMV_MAX_LOGFILE_LINES", 100000);
\OMV\Environment::set("OMV_HTTPREQUEST_TIMEOUT", 60000);
\OMV\Environment::set("OMV_ENGINED_DIR", "/usr/share/openmediavault/engined");
\OMV\Environment::set("OMV_ENGINED_SO_ADDRESS", "/var/lib/openmediavault/engined.sock");
\OMV\Environment::set("OMV_ENGINED_SO_SNDTIMEO", 10);
\OMV\Environment::set("OMV_ENGINED_SO_RCVTIMEO", 180);
\OMV\Environment::set("OMV_ENGINED_SO_CONNECT_MAX_ATTEMPT", 5);
\OMV\Environment::set("OMV_ENGINED_DIRTY_MODULES_FILE", "/var/lib/openmediavault/dirtymodules.json");
\OMV\Environment::set("OMV_USERMGMT_ENUMERATE_USER_ROOT", TRUE);
\OMV\Environment::set("OMV_PLUGIN_ENUMERATE_GET_REPOSITORY", TRUE);
\OMV\Environment::set("OMV_MKFS_OPTIONS_EXT3", "-b 4096 -m 0");
\OMV\Environment::set("OMV_MKFS_OPTIONS_EXT4", "-b 4096 -m 0 -E lazy_itable_init=0,lazy_journal_init=0 -O quota");
\OMV\Environment::set("OMV_MKFS_OPTIONS_JFS", "-q");
\OMV\Environment::set("OMV_MKFS_OPTIONS_XFS", "-f");
\OMV\Environment::set("OMV_MKFS_OPTIONS_BTRFS", "-O skinny-metadata,^mixed-bg");
\OMV\Environment::set("OMV_MKFS_OPTIONS_F2FS", "");
\OMV\Environment::set("OMV_MKFS_EXTRAOPTIONS_EXT4_MD", "-E lazy_itable_init=0,lazy_journal_init=0,stride=128,stripe-width=256");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_EXT2", "defaults,nofail,user_xattr");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_EXT3", "defaults,nofail,user_xattr,usrjquota=aquota.user,grpjquota=aquota.group,jqfmt=vfsv1");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_EXT4", "defaults,nofail,user_xattr,usrquota,grpquota");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_JFS", "defaults,nofail,usrquota,grpquota");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_XFS", "defaults,nofail,usrquota,grpquota");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_EXFAT", "defaults,nofail");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_VFAT", "defaults,nofail");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_NTFS", "defaults,nofail,big_writes");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_HFSPLUS", "defaults,nofail,force");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_BTRFS", "defaults,nofail");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_ISO9660", "ro");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_UDF", "ro");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_BIND", "nofail");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_F2FS", "defaults,nofail,user_xattr,usrjquota=aquota.user,grpjquota=aquota.group,jqfmt=vfsv1"); // https://www.kernel.org/doc/Documentation/filesystems/f2fs.txt
\OMV\Environment::set("OMV_FSTAB_MNTOPS_REISERFS", "defaults,nofail,user_xattr,acl");
\OMV\Environment::set("OMV_FSTAB_MNTOPS_REISER4", "defaults,nofail");
\OMV\Environment::set("OMV_APT_PLUGINS_INDEX_FILE", "/var/lib/openmediavault/apt/pluginsindex.json");
\OMV\Environment::set("OMV_APT_UPGRADE_INDEX_FILE", "/var/lib/openmediavault/apt/upgradeindex.json");
\OMV\Environment::set("OMV_DUNE_QUOTES", [ // http://www.nightsolo.net/dune/god.html
	"The beginning of knowledge is the discovery of something ".
		"we do not understand.",
	"He who controls the spice controls the universe.",
	"Try looking into that place where you dare not look! You'll ".
		"find me there, staring out at you!",
	"A large populace held in check by a small but powerful force ".
		"is quite a common situation in our universe. And we know the ".
		"major conditions wherein this large populace may turn upon ".
		"its keepers: When they find a leader. This is the most ".
		"volatile threat to the powerful; they must retain control of ".
		"leaders. When the populace recognizes its chains. Keep the ".
		"populace blind and unquestioning. When the populace perceives ".
		"a hope of escape from bondage. They must never even believe ".
		"that escape is possible!",
	"I must not fear. Fear is the mind-killer. Fear is the ".
		"little-death that brings total obliteration. I will face my ".
		"fear. I will permit it to pass over me and through me. And ".
		"when it has gone past I will turn the inner eye to see its ".
		"path. Where the fear has gone there will be nothing. Only I ".
		"will remain.",
	"People always fear the unknown, if they are truthful about ".
		"it. But i can face my fears.",
	"Fear is the mind-killer.",
	"Without change something sleeps inside us, and seldom awakens. ".
		"The sleeper must awaken.",
	"What does a mirror look at?",
	"A great man does not seek to lead. He is called to it."
]);

// Read the default file.
$defaultFile = \OMV\Environment::get("OMV_DEFAULT_FILE");
if (posix_access($defaultFile, POSIX_F_OK | POSIX_R_OK)) {
	$lines = file($defaultFile);
	foreach ($lines as $kline => $vline) {
		$regex = "/^(OMV_[A-Z0-9_]+)=(\")?([^\"]+)(\")?$/";
		if (1 == preg_match($regex, trim($vline), $matches))
			\OMV\Environment::set($matches[1], $matches[3]);
		unset($matches);
	}
	unset($lines);
}
