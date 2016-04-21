<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
require_once("openmediavault/globals.inc");

class OMVRpcServiceUsbBackup extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "UsbBackup";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getCandidates");
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("execute");
	}

	/**
	 * Get list of USB volumes that can be used for backup jobs.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   \em uuid, \em devicefile and \em description.
	 *   The field \em uuid is the mount point configuration object UUID.
	 */
	public function getCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get list of all detected filesystems. Note, the list may also
		// contain RAID and LVM volumes, so the list must be filtered.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$objects = $fsbMngr->enumerate();
		// Collect volumes that are candidates for a backup job.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			// Skip filesystem we do not support.
			if (!$fsbMngr->isSupported($objectv['type']))
				continue;
			// Skip the device where the operating system is installed on.
			if (\OMV\System\System::isRootDeviceFile($objectv['devicefile']))
				continue;
			// Get the filesystem backend.
			$fsbMngr->assertBackendExistsByType($objectv['type']);
			$fsb = $fsbMngr->getBackendByType($objectv['type']);
			// Skip filesystems that support union mounts, e.g. UnionFS,
			// aufs or mhddfs. Such filesystems do not have a device file
			// which is required to write the UDEV rule.
			if (!$fsb->hasDeviceFile())
				continue;
			// Get the filesystem implementation.
			$fs = $fsb->getImpl($objectv['devicefile']);
			// Get the storage device containing the filesystem.
			if (FALSE === ($storageDeviceFile = $fs->getStorageDeviceFile()))
				continue;
			\OMV\System\Storage\StorageDevice::assertStorageDeviceExists(
			  $storageDeviceFile);
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
			  $storageDeviceFile);
			// Skip everything which is not an USB or eSATA device or is a
			// read-only medium.
			if (!($sd->isUsb() || $sd->isAta()) || $sd->isReadOnly())
				continue;
			// Prepare result object.
			$result[] = [
				"uuid" => $objectv['uuid'],
				"devicefile" => $objectv['devicefile'],
				"label" => $objectv['label'],
				"type" => $objectv['type'],
				"description" => $sd->getDescription()
			];
		}
		return $result;
	}

	/**
	 * Get list of USB backup job configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.usbbackup.job");
		// Add additional information.
		$objectsAssoc = [];
		foreach ($objects as $objectk => $objectv) {
			// Add new properties.
			$objectv->add("running", "boolean", FALSE);
			$objectv->add("sharedfoldername", "string", gettext("n/a"));
			// Get the shared folder configuration object.
			$sfObject = $db->get("conf.system.sharedfolder",
			  $objectv->get("sharedfolderref"));
			// Update the 'sharedfoldername' property.
			$objectv->set("sharedfoldername", $sfObject->get("name"));
			// Get job status.
			$statusFile = sprintf("/var/run/sync-%s", $objectv->get("fsuuid"));
			$objectv->set("running", file_exists($statusFile));
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a USB backup job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.usbbackup.job", $params['uuid']);
	}

	/**
	 * Set a USB backup job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usbbackup.set");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.usbbackup.job");
		$object->setAssoc($params);
		// Do some validations:
		// - Check if quarantine shared folder exists when it is referenced.
		if ("quarantine" === $object->get("virusaction")) {
			$object2 = $db->get("conf.service.clamav");
			if ($object2->isEmpty("quarantine_sharedfolderref")) {
				throw new \OMV\Exception("No quarantine folder is set.");
			}
		}
		// Set the configuration object.
		$isNew = $object->isNew();
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify($isNew ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.service.usbbackup.job",
		  $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a backup job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function delete($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		return $this->deleteConfigObjectByUuid("conf.service.usbbackup.job",
		  $params['uuid'], "org.openmediavault.conf.service.usbbackup.job");
	}

	/**
	 * Execute a backup job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the cron job to execute.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	public function execute($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Check if the module is marked as dirty. This is an indication
		// that the rsync cron script has not been created or updated
		// until now.
		if ($this->isModuleDirty("usbbackup"))
			throw new \OMV\Config\ConfigDirtyException();
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if ($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Execute the backup script.
			$cmd = "/bin/sh";
			$cmdArgs = [];
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR,
			  $GLOBALS['OMV_USBBACKUP_SCRIPTS_DIR'],
			  sprintf("rsync-%s", $params['uuid']));
			if (FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename))
				throw new \OMV\ExecException($cmd." ".implode(" ", $cmdArgs));
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}
