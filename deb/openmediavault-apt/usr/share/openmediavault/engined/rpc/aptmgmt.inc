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

class AptMgmt extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "AptMgmt";
	}

	public function initialize() {
		$this->registerMethod("getSourceList");
		$this->registerMethod("getSource");
		$this->registerMethod("setSource");
		$this->registerMethod("deleteSource");
	}

	public function getSourceList($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "getlist", [
			"id" => "conf.system.apt.source",
			...$params
		], $context);
	}

	function getSource($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "get", [
			"id" => "conf.system.apt.source",
			"uuid" => $params['uuid']
		], $context);
	}

	function setSource($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "set", [
			"id" => "conf.system.apt.source",
			"data" => $params
		], $context);
	}

	function deleteSource($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "delete", [
			"id" => "conf.system.apt.source",
			"uuid" => $params['uuid']
		], $context);
	}
}
