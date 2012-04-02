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
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/initscript.inc");

class NetworkModule extends OMVModuleAbstract implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "network";
	}

	/**
	 * Create the '/etc/network/interfaces' configuration file.
	 */
	public function mkConf() {
		$cmd = "sudo omv-mkconf interfaces 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create/update the network interface configuration and restart
	 * all network interfaces.
	 */
	public function restartInterfaces() {
		// Create the '/etc/network/interfaces' configuration file.
		$this->mkConf();
		// Restart service
		$initd = new OMVSysVInitScript("networking");
		$initd->restart();
	}

	/**
	 * Create/update the /etc/network/interfaces file and bring up the
	 * given network interface.
	 */
	public function updateInterface($object) {
		// Create the '/etc/network/interfaces' configuration file.
		$this->mkConf();
		// Bring down the given interface to allow reconfiguration
		// if necessary, e.g. change bonding policy.
		$cmd = sprintf("sudo ifdown --force %s 2>&1", $object['devicename']);
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Bring up the given interface (again).
		$cmd = sprintf("sudo ifup --force %s 2>&1", $object['devicename']);
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete an network interface
	 */
	public function deleteInterface($object) {
		// Bring down the network interface.
		$cmd = sprintf("sudo ifdown %s 2>&1", $object['devicename']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Flush the network interface (e.g. remove assigned IP addresses).
		$cmd = sprintf("sudo ip addr flush dev %s 2>&1",
		  $object['devicename']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Create the '/etc/network/interfaces' configuration file.
		$this->mkConf();
	}

	/**
	 * Update the system hostname.
	 */
	public function updateHostname() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//system/network";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Create '/etc/hostname' file
		$cmd = "sudo omv-mkconf hostname 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Set hostname
		$fqdn = $object['hostname'];
		if (!empty($object['domainname'])) {
			$fqdn = sprintf("%s.%s", $object['hostname'],
			  $object['domainname']);
		}
		$cmd = sprintf("sudo hostname %s 2>&1", $fqdn);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create/update the /etc/hosts.allow and /etc/hosts.deny file
	 */
	public function updateHosts() {
		// Update /etc/hosts.allow and /etc/hosts.deny
		$cmd = "sudo omv-mkconf hosts 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create/update the '/etc/network/if-pre-up.d/iptables' file containing
	 * the iptables rules and execute it to apply the rules.
	 */
	public function updateIptables() {
		$cmd = "sudo omv-mkconf iptables 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$cmd = "sudo /etc/network/if-pre-up.d/iptables 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create/update the '/etc/default/halt' file.
	 */
	public function updateHalt() {
		$cmd = "sudo omv-mkconf halt 2>&1";
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
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.dnsnameservers",
		  array($this, "restartInterfaces"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostac",
		  array($this, "updateHosts"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname",
		  array($this, "updateHostname"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname",
		  array($this, "updateHosts"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.interfaces.iface",
		  array($this, "updateInterface"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.iface",
		  array($this, "deleteInterface"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.interfaces.bondiface",
		  array($this, "updateInterface"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.bondiface",
		  array($this, "deleteInterface"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.iptables.rule",
		  array($this, "updateIptables"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.iface",
		  array($this, "updateHalt"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.bondiface",
		  array($this, "updateHalt"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new NetworkModule());
?>
