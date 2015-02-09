<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
require_once("openmediavault/initscript.inc");

abstract class OMVModuleNginxAbstract extends OMVModuleServiceAbstract {
	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		# Build the configuration.
		$cmd = "export LANG=C; omv-mkconf nginx 2>&1";
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		# Test the new configuration.
		$cmd = "export LANG=C; nginx -t 2>&1";
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$initd = new OMVSysVInitScript("nginx", TRUE);
		$initd->invoke("restart");
	}
}

class OMVModuleWebserver extends OMVModuleNginxAbstract
  implements OMVINotifyListener {
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
	public function shouldApply() { return array("certificatemgmt"); }

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
		$xpath = sprintf("//webadmin[sslcertificateref='%s']",
		  $object['uuid']);
		if (TRUE === $xmlConfig->exists($xpath))
			$this->setDirty();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.certificates.sslcertificate",
		  array($this, "onModifySslCertificate"));
	}
}

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleWebserver());
?>
