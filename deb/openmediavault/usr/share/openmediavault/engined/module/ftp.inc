<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
require_once("openmediavault/initscript.inc");
require_once("openmediavault/log.inc");

class OMVModuleFtp extends OMVModuleServiceAbstract
  implements OMVINotifyListener, OMVIModuleServiceStatus, OMVIModuleZeroconf {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "ftp";
	}

	/**
	 * Defines the modules that if present should build their configuration
	 * before the service provided by this module.
	 * @return An array of modules.
	 */
	public function shouldApply() { return array("certificatemgmt"); }

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return array("fstab"); }

	/**
	 * Defines the modules that if present should be stopped after the
	 * service provided by this module.
	 * @return An array of modules.
	 */
	function shouldStop() { return array("fstab"); }

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = "omv-mkconf proftpd 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$initd = new OMVSysVInitScript("proftpd", FALSE);
		$initd->exec();
	}

	/**
	 * Start the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function startService() {
		global $xmlConfig;
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if(FALSE === boolval($object['enable']))
			return;
		$initd = new OMVSysVInitScript("proftpd", TRUE);
		$initd->exec();
	}

	/**
	 * Get the status of the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getStatus() {
		global $xmlConfig;
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$initd = new OMVSysVInitScript("proftpd");
		return array(
			"name" => $this->getName(),
			"title" => gettext("FTP"),
			"enabled" => boolval($object['enable']),
			"running" => ($initd->status() == 0)
		);
	}

	/**
	 * Get the Zeroconf service configuration.
	 */
	public function getZeroconfConfig() {
		return array(
			  "id" => "ftp",
			  "title" => gettext("FTP")
		  );
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
		global $xmlConfig;
		$xpath = sprintf("//services/ftp/shares/share[sharedfolderref='%s']",
		  $object['uuid']);
		if(TRUE === $xmlConfig->exists($xpath))
			$this->setDirty();
		switch($path) {
		case "org.openmediavault.system.shares.sharedfolder":
			$this->setDirtyByName("fstab");
			break;
		default:
			break;
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.ftp.shares.share",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_tls",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "onSharedFolder"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "onSharedFolder"));
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp",
		  array($moduleMgr->getModule("zeroconf"), "setDirty"));
    }
}

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleFtp());

// Register new logfile type.
// Format:
// Oct 17 18:19:00 vmomv5 proftpd[17127]: 192.167.178.101 (::ffff:192.168.178.254[::ffff:192.168.178.254]) - USER test: Login successful.
// Oct 17 18:20:00 vmomv5 proftpd: pam_unix(proftpd:session): session closed for user test
OMVLogFile::registerType("proftpd", array(
  "filename" => "proftpd.log",
  "filepath" => "/var/log/proftpd/proftpd.log",
  "regex" => "/^(\S+\s+\d+\s+\d{2}:\d{2}:\d{2})\s+(\S+)\sproftpd\[\d+\]: (.*)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"M j G:i:s");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"M j G:i:s"));')
	  ),
	  "event" => 3
  )));

// Format:
// <currenttime> <transfertime> <remotehost> <filesize> <filename> <transfertype> <specialactionflag> <direction> <accessmode> <username> <ser­vicename> <authenticationmethod> <authenticateduserid> <completionstatus>
// Thu Oct 17 18:19:00 2013 0 ::ffff:192.168.178.254 56374 /home/ftp/test/testfile b _ i r test ftp 0 * c
OMVLogFile::registerType("proftpd_xferlog", array(
  "filename" => "xferlog",
  "filepath" => "/var/log/proftpd/xferlog",
  "regex" => "/^(\S+\s+\S+\s+\d+\s+\d{2}:\d{2}:\d{2}\s+\d+)\s+(\d+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(a|b)\s+(C|U|T|_)\s+(o|i)\s+(a|g|r)\s+(\S+)\s+(\S+)\s+(0|1)\s+(\d+|\*)\s+(c|i)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"D M d H:i:s Y");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"D M d H:i:s Y"));')
	  ),
	  "transfertime" => 2,
	  "remotehost" => 3,
	  "filesize" => 4,
	  "filename" => 5,
	  "transfertype" => 6,
	  "specialactionflag" => 7,
	  "direction" => 8,
	  "accessmode" => 9,
	  "username" => 10,
	  "ser­vicename" => 11,
	  "authenticationmethod" => 12,
	  "authenticateduserid" => 13,
	  "completionstatus" => 14
  )));
?>
