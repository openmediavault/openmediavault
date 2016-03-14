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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");

class OMVModuleAFP extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus, \OMV\Engine\Module\IZeroconf {
	private $invalidUsers = array("admin");

	/**
	 * Get the module name.
	 */
	public function getName() {
		return "afp";
	}

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return array("monit"); }

	/**
	 * Defines the modules that if present should be stopped after the
	 * service provided by this module.
	 * @return An array of modules.
	 */
	function shouldStop() { return array("monit"); }

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "netatalk");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		// Disable monitoring of this service.
		$monit = new \OMV\System\Monit("netatalk");
		$monit->unmonitor(TRUE);
		// Stop this service and disable the unit file.
		$systemCtl = new \OMV\System\SystemCtl("netatalk");
		$systemCtl->disable(TRUE);
	}

	/**
	 * Start the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_EXEC_FAILED
	 */
	public function startService() {
		global $xmlConfig;
		$xpath = "//services/afp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (FALSE === boolvalEx($object['enable']))
			return;
		// Start this service and enable the unit file.
		$systemCtl = new \OMV\System\SystemCtl("netatalk");
		$systemCtl->enable(TRUE);
		// Enable monitoring of this service.
		$monit = new \OMV\System\Monit("netatalk");
		$monit->monitor();
	}

	/**
	 * Get the status of the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/afp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$monit = new \OMV\System\Monit("netatalk");
		return array(
			"name" => $this->getName(),
			"title" => gettext("Apple Filing"),
			"enabled" => boolvalEx($object['enable']),
			"running" => $monit->isRunning()
		);
	}

	/**
	 * Get the Zeroconf service configuration.
	 */
	public function getZeroconfConfig() {
		return array(
			  "id" => "afp",
			  "title" => gettext("Apple Filing")
		  );
	}

	/**
	 * Helper function that is called whenever a share is created, modified
	 * or deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onShare($type, $path, $object) {
		// Set this module to dirty.
		$this->setDirty();
		// If this share is used for time machine then set the 'zeroconf'
		// module to dirty to update the Zeroconf/Bonjour configuration.
		if (array_boolval($object['options'], 'tm'))
			$this->setDirtyByName("zeroconf");
	}

	/**
	 * Helper function to find out whether the given shared folder
	 * configuration object is used. If it is used, then mark the
	 * module as dirty.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onSharedFolder($type, $path, $object) {
		$db = &\OMV\Config\Database::getInstance();
		if (TRUE === $db->getBackend()->exists(sprintf(
		  "//services/afp/shares/share[sharedfolderref='%s']",
		  $object['uuid'])))
			$this->setDirty();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.afp",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.afp.shares.share",
		  array($this, "onShare"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "onSharedFolder"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "onSharedFolder"));
		$moduleMngr = &\OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.afp",
		  array($moduleMngr->getModule("zeroconf"), "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.afp",
		  array($moduleMngr->getModule("monit"), "setDirty"));
    }
}
