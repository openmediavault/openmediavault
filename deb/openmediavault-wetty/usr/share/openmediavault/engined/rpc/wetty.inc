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
namespace Engined\Rpc;

require_once("openmediavault/functions.inc");

class OMVRpcServiceWeTTY extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "WeTTY";
	}

	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
	}

	function get($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "get", [
			"id" => "conf.service.wetty"
		], $context);
	}

	function set($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Do some validations.
		if (boolvalEx($params['enable'])) {
			$db = \OMV\Config\Database::getInstance();
			$object = $db->get("conf.service.ssh");
			$object->assertValue("enable", TRUE,
				"SSH must be enabled.");
			$object->assertValue("passwordauthentication", TRUE,
				"SSH password authentication must be enabled.");
		}
		return \OMV\Rpc\Rpc::call("Config", "set", [
			"id" => "conf.service.wetty",
			"data" => $params
		], $context);
	}
}
