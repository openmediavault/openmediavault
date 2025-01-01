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

class MinIO extends \OMV\Engine\Module\ServiceAbstract
		implements \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	public function getName() {
		return "minio";
	}

	public function deployAfter(): array {
		return ["certificates"];
	}

	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.minio");
		$systemCtl = new \OMV\System\SystemCtl("pod-minio");
		return [
			"name" => $this->getName(),
			"title" => gettext("S3"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		];
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.minio",
			[$this, "setDirty"]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.minio",
			[$moduleMngr->getModule("zeroconf"), "setDirty"]);
	}
}
