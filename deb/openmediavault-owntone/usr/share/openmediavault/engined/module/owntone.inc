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

class OwnTone extends \OMV\Engine\Module\ServiceAbstract
		implements \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	public function getName() {
		return "owntone";
	}

	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.owntone");
		$systemCtl = new \OMV\System\SystemCtl("container-owntone");
		return [
			"name" => $this->getName(),
			"title" => gettext("OwnTone"),
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
	final public function onModifySharedFolder($type, $path, $object) {
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $db->exists("conf.service.owntone", [
			"operator" => "stringEquals",
			"arg0" => "sharedfolderref",
			"arg1" => $object['uuid']
		])) {
			$this->setDirty();
		}
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.owntone",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder",
			[$this, "onModifySharedFolder"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.time",
			[$this, "setDirty"]);
	}
}
