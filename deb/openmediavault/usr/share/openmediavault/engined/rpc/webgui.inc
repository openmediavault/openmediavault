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

class WebGui extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "WebGui";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("dismissWelcomeMessage");
		$this->registerMethod("getLocalStorageItems");
		$this->registerMethod("setLocalStorageItem");
		$this->registerMethod("clearLocalStorageItems");
	}

	/**
	 * Get settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.webadmin");
		return $object->getAssoc();
	}

	/**
	 * Set settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.webgui.setsettings");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.webadmin");
		$object->setAssoc($params);
		// Update the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Dismiss the welcome message which is displayed at the first log in.
	 */
	function dismissWelcomeMessage($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		unlink("/var/lib/openmediavault/display_welcome_message");
	}

	/**
	 * Get the local storage settings of the current context user.
	 * @param params An array containing the following fields:
	 *   \em devicetype The device type, e.g. `desktop` or `mobile`.
	 * @param context The context of the caller.
	 * @return void
	 */
	function getLocalStorageItems($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.webgui.getlocalstorageitems");
		$content = [];
		$filename = build_path(DIRECTORY_SEPARATOR,
			\OMV\Environment::get("OMV_WEBGUI_LOCALSTORAGE_DIR",
				"/var/lib/openmediavault/workbench/localstorage.d"),
			$context["username"]);
		$jsonFile = new \OMV\Json\File($filename);
		if ($jsonFile->exists($filename)) {
			$jsonFile->open("r", LOCK_SH);
			$content = $jsonFile->read();
			$jsonFile->close();
		}
		return array_value($content, $params['devicetype'], []);
	}

	/**
	 * Set the local storage settings of the current context user.
	 * @param params An array containing the following fields:
	 *   \em devicetype The device type, e.g. `desktop` or `mobile`.
	 *   \em key A string containing the name of the key you want to
	 *     create/update.
	 *   \em value A string containing the value you want to give
	 *     the key.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function setLocalStorageItem($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.webgui.setlocalstorageitem");
		$content = [];
		$filename = build_path(DIRECTORY_SEPARATOR,
			\OMV\Environment::get("OMV_WEBGUI_LOCALSTORAGE_DIR",
				"/var/lib/openmediavault/workbench/localstorage.d"),
			$context["username"]);
		$jsonFile = new \OMV\Json\File($filename);
		$jsonFile->open("c+");
		if (!$jsonFile->isEmpty()) {
			$content = $jsonFile->read();
		}
		$key = sprintf("%s.%s", $params['devicetype'], $params['key']);
		$contentDict = new \OMV\Dictionary($content);
		$contentDict->set($key, $params['value']);
		$jsonFile->write($contentDict->getData());
		$jsonFile->close();
		return $params['data'];
	}

	/**
	 * Clear the local storage settings of the current context user.
	 * @param params An array containing the following fields:
	 *   \em devicetype The device type, e.g. `desktop` or `mobile`.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function clearLocalStorageItems($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.webgui.clearlocalstorageitems");
		$filename = build_path(DIRECTORY_SEPARATOR,
			\OMV\Environment::get("OMV_WEBGUI_LOCALSTORAGE_DIR",
				"/var/lib/openmediavault/workbench/localstorage.d"),
			$context["username"]);
		$jsonFile = new \OMV\Json\File($filename);
		if ($jsonFile->exists($filename)) {
			$jsonFile->open("c+");
			$contentDict = new \OMV\Dictionary($jsonFile->read());
			$contentDict->remove($params['devicetype']);
			$jsonFile->write($contentDict->getData());
			$jsonFile->close();
		}
	}
}
