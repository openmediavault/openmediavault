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

class PhotoPrism extends \OMV\Engine\Module\ServiceAbstract
		implements \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	public function getName() {
		return "photoprism";
	}

	public function deployAfter(): array {
		return ["certificates"];
	}

	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.photoprism");
		$systemCtl = new \OMV\System\SystemCtl("pod-photoprism");
		return [
			"name" => $this->getName(),
			"title" => gettext("PhotoPrism"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		];
	}

	final public function onModifySharedFolder($type, $path, $object) {
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $db->exists("conf.service.photoprism", [
			"operator" => "or",
			"arg0" => [
				"operator" => "stringEquals",
				"arg0" => "originals_sharedfolderref",
				"arg1" => $object['uuid']
			],
			"arg1" => [
				"operator" => "or",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "appdata_sharedfolderref",
					"arg1" => $object['uuid']
				],
				"arg1" => [
					"operator" => "stringEquals",
					"arg0" => "import_sharedfolderref",
					"arg1" => $object['uuid']
				]
			]
		])) {
			$this->setDirty();
		}
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.photoprism",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder",
			[$this, "onModifySharedFolder"]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.photoprism",
			[$moduleMngr->getModule("zeroconf"), "setDirty"]);
	}
}
