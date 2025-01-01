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

class Ssh extends \OMV\Engine\Module\ServiceAbstract implements
		\OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	public function getName() {
		return "ssh";
	}

	public function getDescription() {
		return gettext("A secure shell (SSH) server.");
	}

	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ssh");
		$systemCtl = new \OMV\System\SystemCtl("ssh");
		return [
			"name" => $this->getName(),
			"title" => gettext("SSH"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		];
	}

	/**
	 * Helper function that is called whenever a user has been created,
	 * modified or deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onUser($type, $path, $object, $oldObject = NULL) {
		switch ($type) {
		case OMV_NOTIFY_CREATE:
		case OMV_NOTIFY_DELETE:
			// Set the module to dirty when public keys are defined.
			if (!empty($object['sshpubkeys'])) {
				$this->setDirty();
			}
			break;
		case OMV_NOTIFY_MODIFY:
			// If the 'sshpubkey' is set, then set this module to dirty.
			// Note, it would be better to set the module to dirty only
			// if the 'sshpubkey' field has been modified, but the
			// current framework does not have the capability to do that.
			if (!empty($object['sshpubkeys'])) {
				$this->setDirty();
			} else {
				// Check whether the previous/old user object had one or
				// more SSH public keys that have been removed now.
				if ($oldObject && !empty($oldObject['sshpubkeys'])) {
					$this->setDirty();
				}
			}
			break;
		default:
			// Nothing to do here.
			break;
		}
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.ssh",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.usermngmnt.user",
			[$this, "onUser"]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.ssh",
			[$moduleMngr->getModule("zeroconf"), "setDirty"]);
	}
}
