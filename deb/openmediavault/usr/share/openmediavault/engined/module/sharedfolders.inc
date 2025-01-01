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

class Sharedfolders extends \OMV\Engine\Module\ServiceAbstract
		implements \OMV\Engine\Notify\IListener {
	public function getName() {
		return "sharedfolders";
	}

	public function deployAfter(): array {
		return ["systemd", "task"];
	}

	public function deploy() {
		// Nothing to do here.
	}

	/**
	 * Helper function to add a new task when a shared folder has been deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onDeleteSharedFolder($type, $path, $object) {
		\OMV\Salt\Task::create("omv.task.sharedfolder_alias.disable", [
			"name" => $object['name']
		]);
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.sharedfolder",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.sharedfolder",
			[$this, "onDeleteSharedFolder"]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.sharedfolder",
			[$moduleMngr->getModule("systemd"), "setDirty"]);
	}
}
