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
require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/lvm.inc");

class OMVLogicalVolumeMgmtModule extends OMVModuleAbstract
  implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "logicalvolumemgmt";
	}

	/**
	 * Create a physical volume.
	 * @param object The physical volume object.
	 */
	public function createPhysicalVolume($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Make sure the disk device is clean, no partition table
		// should exist, otherwise pvcreate fails.
		$cmd = sprintf("sudo dd if=/dev/zero of=%s bs=512 count=1 2>&1",
		  $object['devicefile']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Create the physical volume.
		$cmd = sprintf("sudo pvcreate %s 2>&1", $object['devicefile']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete a physical volume.
	 * @param object The physical volume object.
	 */
	public function deletePhysicalVolume($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmd = sprintf("sudo pvremove %s 2>&1", $object['devicefile']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create a volume group.
	 * @param object The volume group object.
	 */
	public function createVolumeGroup($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmd = sprintf("sudo vgcreate %s %s 2>&1", $object['name'],
		  implode(" ", $object['devices']));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete a volume group.
	 * @param object The volume group object.
	 */
	public function deleteVolumeGroup($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmd = sprintf("sudo vgremove %s 2>&1", $object['name']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Modify a volume group.
	 * @param object The volume group object.
	 */
	public function modifyVolumeGroup($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$vg = new OMVVolumeGroup($object['devicefile']);
		// Check if the 'devices' field is set. Extend and/or reduce the
		// volume group based on new devices configuration.
		if (isset($object['devices'])) {
			// Get the physical volumes currently assigned to the volume group.
			$assignedPV = $vg->getPVName();
			// Get the removed physical volumes.
			$reducePV = array_diff($assignedPV, $object['devices']);
			// Get the new assigned physical volumes.
			$extendPV = array_diff($object['devices'], $assignedPV);
			// Execute commands to add and remove physical volumes to or from
			// the volume group.
			if (!empty($reducePV)) {
				$cmd = sprintf("sudo vgreduce %s %s 2>&1", $vg->getName(),
				  implode(" ", $reducePV));
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			}
			if (!empty($extendPV)) {
				$cmd = sprintf("sudo vgextend %s %s 2>&1", $vg->getName(),
				  implode(" ", $extendPV));
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			}
		}
		// Check if the 'name' field is set and whether it is different to
		// the current one. If so, then rename the volume group.
		if (isset($object['name'])) {
			if ($object['name'] !== $vg->getName()) {
				$cmd = sprintf("sudo vgrename %s %s 2>&1", $vg->getUuid(),
				  $object['name']);
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			}
		}
	}

	/**
	 * Create a logical volume.
	 * @param object The logical volume object.
	 */
	public function createLogicalVolume($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmd = sprintf("sudo lvcreate -L %d%s -n %s %s 2>&1", $object['size'],
		  $object['unit'], $object['name'], $object['vgname']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete a logical volume.
	 * @param object The logical volume object.
	 */
	public function deleteLogicalVolume($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmd = sprintf("sudo lvremove --force %s 2>&1", $object['devicefile']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Modify a logical volume.
	 * @param object The logical volume object.
	 */
	public function modifyLogicalVolume($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$lv = new OMVLogicalVolume($object['devicefile']);
		// If the 'size' and 'unit' fields are set, then extend or reduce the
		// logical volume.
		if (TRUE === array_keys_exists(array("size", "unit"), $object)) {
			$size = binary_convert($lv->getSize(), "B", $object['unit']);
			if ($size < $object['size']) {
				$cmd = sprintf("sudo lvextend -L %d%s %s 2>&1",
				  $object['size'], $object['unit'], $object['devicefile']);
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			} else if ($size > $object['size']) {
				$cmd = sprintf("sudo lvreduce -L %d%s %s 2>&1",
				  $object['size'], $object['unit'], $object['devicefile']);
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			}
		}
		// Check if the 'name' field is set and whether it is different to
		// the current one. If so, then rename the volume group.
		if (isset($object['name'])) {
			if ($object['name'] !== $lv->getName()) {
				$cmd = sprintf("sudo lvrename %s %s 2>&1", $lv->getPath(),
				  $object['name']);
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			}
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.physicalvolume",
		  array($this, "createPhysicalVolume"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.physicalvolume",
		  array($this, "deletePhysicalVolume"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.volumegroup",
		  array($this, "createVolumeGroup"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.volumegroup",
		  array($this, "deleteVolumeGroup"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.volumegroup",
		  array($this, "modifyVolumeGroup"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.logicalvolume",
		  array($this, "deleteLogicalVolume"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.logicalvolume",
		  array($this, "modifyLogicalVolume"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.logicalvolume",
		  array($this, "createLogicalVolume"), 1);
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVLogicalVolumeMgmtModule());
?>
