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
namespace Engined\Module;

class Proftpd extends \OMV\Engine\Module\ServiceAbstract implements
		\OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	public function getName() {
		return "proftpd";
	}

	public function getAlias() {
		return "ftp";
	}

	public function getDescription() {
		return gettext("A powerful modular FTP/SFTP/FTPS server.");
	}

	public function deployAfter(): array {
		return ["certificates", "monit", "timezone"];
	}

	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ftp");
		$systemCtl = new \OMV\System\SystemCtl("proftpd");
		return [
			"name" => $this->getName(),
			"title" => gettext("FTP"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		];
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
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $db->exists("conf.service.ftp.share", [
			"operator" => "stringEquals",
			"arg0" => "sharedfolderref",
			"arg1" => $object['uuid']
		])) {
			$this->setDirty();
		}
	}

	/**
	 * Helper function that is called whenever the global service
	 * settings have been modified.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The new configuration object.
	 * @param oldObject The previous configuration object.
	 */
	final public function onModifySettings($type, $path, $object, $oldObject) {
		// Mark this module as dirty.
		$this->setDirty();
		// Mark 'monit' as dirty when this service is enabled/disabled.
		if ($oldObject['enable'] !== $object['enable']) {
			$this->setDirtyByName("monit");
		}
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.ftp",
			[$this, "onModifySettings"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.ftp.share",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.ftp.modban.rule",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.ftp.modtls",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.network.dns",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.usermanagement.homedirectory",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder",
			[$this, "onSharedFolder"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder.privilege",
			[$this, "onSharedFolder"]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.ftp",
			[$moduleMngr->getModule("zeroconf"), "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.ftp.modtls",
			[$moduleMngr->getModule("monit"), "setDirty"]);
	}
}
