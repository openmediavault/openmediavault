<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
abstract class OMVModuleNginxAbstract
  extends \OMV\Engine\Module\ServiceAbstract {
	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		// Build the configuration.
		$cmd = new \OMV\System\Process("omv-mkconf", "nginx");
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Ensure the control panel configuration is enabled.
		$cmd = new \OMV\System\Process("nginx_ensite",
		  "openmediavault-webgui");
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Test the new configuration.
		$cmd = new \OMV\System\Process("nginx", "-t");
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$systemCtl = new \OMV\System\SystemCtl("nginx");
		$systemCtl->restart();
	}
}

class OMVModuleWebserver extends OMVModuleNginxAbstract
  implements \OMV\Engine\Notify\IListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "webserver";
	}

	/**
	 * Defines the modules that if present should build their configuration
	 * before the service provided by this module.
	 * @return An array of modules.
	 */
	public function shouldApply() { return [ "certificatemgmt" ]; }

	/**
	 * Helper function to find out whether the given SSL certificate
	 * is used by this service. If it is used, then mark the module as
	 * dirty.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onModifySslCertificate($type, $path, $object) {
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $db->exists("conf.webadmin", [
			"operator" => "stringEquals",
			"arg0" => "sslcertificateref",
			"arg1" => $object['uuid']
		])) {
			$this->setDirty();
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.certificate.ssl",
		  [ $this, "onModifySslCertificate" ]);
	}
}
