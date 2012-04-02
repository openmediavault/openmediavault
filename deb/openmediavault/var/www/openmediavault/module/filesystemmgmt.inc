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
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class FileSystemMgmtModule extends OMVModuleAbstract implements
  OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "filesystemmgmt";
	}

	/**
	 * Create the filesystem.
	 * @param object The filesystem object.
	 */
	public function create($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmdargs = sprintf("-b -t %s", $object['type']);
		if (!empty($object['label'])) {
			$cmdargs .= sprintf(" -L %s", escapeshellarg($object['label']));
		}
		$cmd = sprintf("sudo omv-initfs %s %s >/dev/null 2>&1 &", $cmdargs,
		  escapeshellarg($object['devicefile']));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Wipe the given filesystem. Unmount the filesystem and delete the
	 * associated mount point and quota configuration if necessary.
	 * @param object The filesystem object.
	 */
	public function delete($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Note, the UUID may be empty in some cases (e.g. iso9660), so use
		// the devicefile instead.
		$fs = new OMVFilesystem($object['devicefile']);
		// Unmount the filesystem if necessary.
		if (TRUE === $fs->isMounted()) {
			$cmd = sprintf("sudo umount -vlf %s >/dev/null 2>&1",
			  $fs->getDeviceFile());
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
		// Delete the associated fstab mount point entry if necessary.
		$xpath = sprintf("//system/fstab/mntent[fsname='%s']",
		  $fs->getUuid());
		$meObject = $xmlConfig->get($xpath);
		if (is_null($meObject)) {
			// There is no mount point configured for the given filesystem
			// UUID, so try the devicefile instead.
			$xpath = sprintf("//system/fstab/mntent[fsname='%s']",
			  $fs->getDeviceFile());
			$meObject = $xmlConfig->get($xpath);
		}
		if (!is_null($meObject)) {
			// A mount point configuration has been found, so delete it now.
			OMVRpc::exec("FsTab", "delete", array("uuid" => $meObject['uuid']));
		}
		// Finally erase the filesystem. Whether the partition type is 'dos'
		// then it is necessary to erase the MBR before, otherwise 'wipefs'
		// fails, e.g.:
		// wipefs: error: /dev/sdh1: appears to contain 'dos' partition table
		$cmd = sprintf("sudo wipefs -a %s 2>&1", $fs->getDeviceFile());
		if (in_array($fs->getPartitionType(), array("dos", "vfat"))) {
			// http://en.wikipedia.org/wiki/Master_boot_record
			$cmd = sprintf("sudo dd if=/dev/zero of=%s bs=512 count=1 ".
			  "2>&1; %s", $fs->getDeviceFile(), $cmd);
		}
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.filesystem",
		  array($this, "create"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.filesystem",
		  array($this, "delete"));
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new FileSystemMgmtModule());
?>
