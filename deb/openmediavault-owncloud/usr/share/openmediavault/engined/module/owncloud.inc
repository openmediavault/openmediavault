<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/error.inc");

class OMVModuleOwnCloud extends OMVModuleServiceAbstract
  implements OMVINotifyListener, OMVIModuleZeroconf {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "owncloud";
	}

	/**
	 * Get the Zeroconf service configuration.
	 */
	public function getZeroconfConfig() {
		return array(
			  "id" => "owncloud",
			  "title" => gettext("ownCloud")
		  );
	}

	/**
	 * Defines the modules that if present should start after the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function startBefore() { return array("webserver"); }

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		// Create the configuration.
		$cmd = "omv-mkconf owncloud 2>&1";
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update the shared folder ACL to allow the web server to access
		// the shared folders.
		$this->execTasks("setfacl");
	}

	/**
	 * Helper function that is called whenever the basic service settings
	 * have been modified.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The new configuration object.
	 * @param oldObject The previous configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	final public function onModifySettings($type, $path, $object, $oldObject) {
		// Mark this module as dirty.
		$this->setDirty();
		// Check whether the webserver needs to be restarted because
		// ownCloud has been enabled/disabled. In this case mark the
		// module as dirty.
		if (boolval($oldObject['enable']) != boolval($object['enable'])) {
			$this->setDirtyByName("webserver");
		}
	}

	/**
	 * Helper function to find out whether the given shared folder
	 * configuration object is used. If it is used, then mark the
	 * module as dirty.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onModifySharedFolder($type, $path, $object) {
		global $xmlConfig;
		$xpath = sprintf("//services/owncloud/shares/share[".
		  "sharedfolderref='%s']", $object['uuid']);
		if (TRUE === $xmlConfig->exists($xpath))
			$this->setDirty();
	}

	/**
	 * Helper function to find out whether the given SSL certificate
	 * is used by this service. If it is used, then mark the module as
	 * dirty.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onModifySslCertificate($type, $path, $object) {
		global $xmlConfig;
		$xpath = sprintf("//services/owncloud[sslcertificateref='%s']",
		  $object['uuid']);
		if (TRUE === $xmlConfig->exists($xpath))
			$this->setDirtyByName("webserver");
	}

	/**
	 * Helper function to add a new task when a share has been added. The
	 * task will ensure that the web server can access the shared folder
	 * by adding r/w permissions for the user 'www-data'.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @return None.
	 */
	final public function onCreateShare($type, $path, $object) {
		global $xmlConfig;
		// Only process local shared folders.
		if ($object['type'] !== "localfs")
			return;
		// Only modify the shared folder ACL if this is added the
		// first time by an ownCloud share.
		$xpath = sprintf("//services/owncloud/shares/share[".
		  "sharedfolderref='%s']", $object['sharedfolderref']);
		if (1 < $xmlConfig->count($xpath))
			return;
		// Add a new task that modifies the shared folder ACL.
		$this->addTask("setfacl", "setSharedFolderFacl", array(
			"mode" => "modify",
			"object" => $object
		));
	}

	/**
	 * Helper function to add a new task when a share has been remoeved. The
	 * task will ensure that the web server permissions are removed from the
	 * shared folder.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @return None.
	 */
	final public function onDeleteShare($type, $path, $object) {
		global $xmlConfig;
		// Only process local shared folders.
		if ($object['type'] !== "localfs")
			return;
		// Only modify/remove the shared folder ACL if this is not still
		// used by other ownCloud shares.
		$xpath = sprintf("//services/owncloud/shares/share[".
		  "sharedfolderref='%s']", $object['sharedfolderref']);
		if (1 < $xmlConfig->count($xpath))
			return;
		// Add a new task that modifies the shared folder ACL.
		$this->addTask("setfacl", "setSharedFolderFacl", array(
			"mode" => "remove",
			"object" => $object
		));
	}

	/**
	 * Helper method to set the ACL for the given shared folder.
	 * @param params The method parameters.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	final public function setSharedFolderFacl($params) {
		global $xmlConfig;
		// Get the shared folder configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['object']['sharedfolderref']);
		$sfObject = $xmlConfig->get($xpath);
		if (is_null($sfObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get the shared folder's mount point configuration object.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $sfObject['mntentref']);
		$meObject = $xmlConfig->get($xpath);
		if (is_null($meObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Build the command argument list and set the file ACL.
		$cmdargs = array();
		$cmdargs[] = "--recursive";
		switch ($params['mode']) {
		case "modify":
			$cmdargs[] = "--modify";
			$cmdargs[] = "default:user:www-data:rwx,user:www-data:rwx";
			break;
		case "remove":
			$cmdargs[] = "--remove";
			$cmdargs[] = "default:user:www-data,user:www-data";
			break;
		}
		$cmd = sprintf("export LANG=C; setfacl %s -- %s 2>&1",
		  implode(" ", $cmdargs), escapeshellarg(sprintf("%s/%s",
		  $meObject['dir'], $sfObject['reldirpath'])));
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.owncloud",
		  array($this, "onModifySettings"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.owncloud.shares.share",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.services.owncloud.shares.share",
		  array($this, "onCreateShare"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.owncloud.shares.share",
		  array($this, "onDeleteShare"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "onModifySharedFolder"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "onModifySharedFolder"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.certificates.sslcertificate",
		  array($this, "onModifySslCertificate"));
		// Changing webgui settings requires recreation of ownCloud
		// configuration.
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.webadmin",
		  array($moduleMgr->getModule("webserver"), "setDirty"));
	}
}

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleOwnCloud());
?>
