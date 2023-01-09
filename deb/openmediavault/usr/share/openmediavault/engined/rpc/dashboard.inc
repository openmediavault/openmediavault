<?php

/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2023 Volker Theile
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

use stdClass;

require_once("openmediavault/functions.inc");

class Dashboards extends \OMV\Rpc\ServiceAbstract
{
	/**
	 * Get the RPC service name.
	 */
	public function getName()
	{
		return "Dashboard";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize()
	{
		$this->registerMethod("getDashboardSetting");
		$this->registerMethod("setDashboardSetting");
		$this->registerMethod("deleteDashboardSetting");
	}

	/**
	 * Get the dashbaord configuration of the current context user.
	 * @param params An array containing the following fields:
	 *   \em useragent The useragent of the current user.
	 * @param context The context of the caller.
	 * @return void
	 */
	function getDashboardSetting($params, $context)
	{
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);

		// Validate the parameters of the RPC service method.
		$this->validateMethodParams(
			$params,
			"rpc.dashboard.getsetting"
		);

		$result = [];
		$filename = sprintf(
			"%s%s%s.json",
			\OMV\Environment::get("OMV_WEBGUI_DASHBOARD_JSON_DIR"),
			DIRECTORY_SEPARATOR,
			$context["username"]
		);
		$jsonFile = new \OMV\Json\File($filename);
		if ($jsonFile->exists($filename)) {
			$jsonFile->open("r");
			$result = $jsonFile->read();
			$jsonFile->close();
		}
		return $result[$params['deviceType']];
	}

	public function setDashboardSetting($params, $context)
	{
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);

		// Validate the parameters of the RPC service method.
		$this->validateMethodParams(
			$params,
			"rpc.dashboard.setsetting"
		);

		$result = [$params['deviceType'] => new stdClass()];
		$filename = sprintf(
			"%s%s%s.json",
			\OMV\Environment::get("OMV_WEBGUI_DASHBOARD_JSON_DIR"),
			DIRECTORY_SEPARATOR,
			$context["username"]
		);
		$jsonFile = new \OMV\Json\File($filename);
		if ($jsonFile->exists($filename)) {
			$jsonFile->open("r");
			$result = $jsonFile->read();
			$jsonFile->close();
		}
		$result[$params['deviceType']] = $params['data'];
		$jsonFile->open();
		$result = $jsonFile->write($result);
		$jsonFile->close();
		return $params['data'];
	}

	public function deleteDashboardSetting($params, $context)
	{
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);

		// Validate the parameters of the RPC service method.
		$this->validateMethodParams(
			$params,
			"rpc.dashboard.deletesetting"
		);

		$filename = sprintf(
			"%s%s%s.json",
			\OMV\Environment::get("OMV_WEBGUI_DASHBOARD_JSON_DIR"),
			DIRECTORY_SEPARATOR,
			$context["username"]
		);
		$jsonFile = new \OMV\Json\File($filename);
		if ($jsonFile->exists($filename)) {
			$jsonFile->open("r+");
			$result = $jsonFile->read();
			unset($result[$params['deviceType']]);
			$jsonFile->truncate(0);
			$result = $jsonFile->write($result);
			$jsonFile->close();
		}
	}
}
