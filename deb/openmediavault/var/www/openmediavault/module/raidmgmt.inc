<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class RaidMgmtModule extends OMVModuleAbstract implements
  OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "raidmgmt";
	}

	/**
	 * Create the mdadm configuration file.
	 */
	public function mkConf() {
		$cmd = "sudo omv-mkconf mdadm 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create a RAID device.
	 * @param object The RAID device object.
	 */
	public function create($object) {
		$cmdArgs = sprintf("%s -l %s -n %d", $object['devicefile'],
		  $object['level'], $object['numdevices']);
		if (!empty($object['name'])) {
			$cmdArgs .= sprintf(" -N %s", $object['name']);
		}
		$cmdArgs .= " " . implode(" ", $object['devices']);
		$cmd = sprintf("sudo omv-mkraid %s 2>&1", $cmdArgs);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update the mdadm configuration file.
		$this->mkConf();
	}

	/**
	 * Grow a RAID device.
	 * @param object The RAID device object.
	 */
	public function grow($object) {
		// Get RAID device details
		$raid = new OMVRaidDevice($object['devicefile']);
		if (FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device '%s' not found", $raid->getDeviceFile()));
		}
		$numDevices = $raid->getNumDevices();
		$devices = $raid->getDevices();
		// Extract the new devices and add them to the RAID device.
		$newDevices = array_diff($object['devices'], $devices);
		$cmd = sprintf("sudo mdadm --add %s %s 2>&1", $raid->getDeviceFile(),
		  implode(" ", $newDevices));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Now grow the RAID.
		// See https://raid.wiki.kernel.org/index.php/Growing
		$cmd = sprintf("sudo mdadm --grow --raid-devices=%d %s 2>&1",
		  $object['numdevices'], $raid->getDeviceFile());
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update the mdadm configuration file.
		$this->mkConf();
	}

	/**
	 * Delete a RAID device.
	 * @param object The RAID device object.
	 */
	public function delete($object) {
		$cmd = sprintf("sudo omv-rmraid %s", $object['devicefile']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update the mdadm configuration file.
		$this->mkConf();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.raid.device",
		  array($this, "create"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.raid.device",
		  array($this, "grow"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.raid.device",
		  array($this, "delete"));
		// Update config file if notification/email configuration has
		// been modified.
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.email",
		  array($this, "mkConf"));
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new RaidMgmtModule());
?>
