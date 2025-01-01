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
namespace Engined\Rpc;

class Smb extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "SMB";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getShareList");
		$this->registerMethod("getShare");
		$this->registerMethod("setShare");
		$this->registerMethod("deleteShare");
		$this->registerMethod("emptyRecycleBin");
		$this->registerMethod("getStats");
	}

	/**
	 * Get the configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smb");
		// Remove useless properties from the object.
		$object->remove("shares");
		return $object->getAssoc();
	}

	/**
	 * Set the configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.smb.setsettings");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smb");
		$object->setAssoc($params);
		// Do some validations:
		if ($object->get("enable")) {
			// - Check if the hostname is not longer than 15 characters.
			//   https://wiki.samba.org/index.php/Active_Directory_Naming_FAQ#NetBIOS_Names
			$dnsObject = $db->get("conf.system.network.dns");
			if (strlen($dnsObject->get("hostname")) > 15) {
				throw new \OMV\ConstraintException(
					"The DNS hostname is limited to 15 characters when using SMB/CIFS. ".
					"Please change the hostname in the network settings page.");
			}
			// - Check if the user home directories are enabled.
			if ($object->get("homesenable")) {
				$hdObject = $db->get("conf.system.usermngmnt.homedir");
				if (FALSE === $hdObject->get("enable")) {
					throw new \OMV\ConstraintException("Can't share user home ".
						"directories because they are not enabled.");
				}
			}
		}
		// Set the configuration object.
		$db->set($object);
		// Remove useless properties from the object.
		$object->remove("shares");
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get list of share configuration objects.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getShareList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.smb.share");
		// Add additional share informations.
		$objectsAssoc = [];
		foreach ($objects as $objectk => &$objectv) {
			// Add the new property 'sharedfoldername'.
			$objectv->add("sharedfoldername", "string", gettext("n/a"));
			// Get the shared folder configuration object.
			$sfObject = $db->get("conf.system.sharedfolder",
			  $objectv->get("sharedfolderref"));
			// Update the 'sharedfoldername' property.
			$objectv->set("sharedfoldername", $sfObject->get("name"));
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a share configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.smb.share", $params['uuid']);
	}

	/**
	 * Set a share configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.smb.setshare");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.smb.share");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		// Check uniqueness:
		// - Shared folder
		$db->assertIsUnique($object, "sharedfolderref");
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a share configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smb.share", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Empty the recycle bin of the given shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	public function emptyRecycleBin($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Check if the module is marked as dirty. This can be an indication
		// that the samba recycle bin cron script has not been created or
		// updated until now.
		if ($this->isModuleDirty("samba"))
			throw new \OMV\Config\ConfigDirtyException();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// Execute the rsync cron script.
			$cmdArgs = [];
			$cmdArgs[] = "--shell";
			$cmdArgs[] = "--non-interactive";
			$cmdArgs[] = "--";
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR,
			  \OMV\Environment::get("OMV_CRONSCRIPTS_DIR"),
			  sprintf("samba-recycle-%s", $params['uuid']));
			$cmdArgs[] = "-f";
			$cmd = new \OMV\System\Process("sudo", $cmdArgs);
			$cmd->setEnv("SHELL", "/bin/sh");
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}

	/**
	 * Get statistics.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A string containing the requested statistics.
	 */
	public function getStats($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Raise an exception if the module is marked as dirty,
		// otherwise the 'smbstatus' command will return incorrect
		// information because the SMB service has not been deployed
		// until now.
		if ($this->isModuleDirty("samba")) {
			throw new \OMV\Config\ConfigDirtyException();
		}
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smb");
		if (FALSE === $object->get("enable")) {
			$stats = gettext("Service disabled");
		} else {
			$cmdArgs = [];
			$cmdArgs[] = "-s";
			$cmd = new \OMV\System\Process("smbstatus", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			array_shift($output);
			array_shift($output);
			$stats = implode("\n", $output);
		}
		return $stats;
	}
}
