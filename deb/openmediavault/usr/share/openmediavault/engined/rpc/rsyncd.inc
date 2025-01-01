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

class Rsyncd extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Rsyncd";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getModuleList");
		$this->registerMethod("getModule");
		$this->registerMethod("setModule");
		$this->registerMethod("deleteModule");
	}

	/**
	 * Get settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.rsyncd");
		// Remove useless properties from the object.
		$object->remove("modules");
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
		$this->validateMethodParams($params, "rpc.rsyncd.setsettings");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.rsyncd");
		$object->setAssoc($params);
		// Set the configuration object.
		$db->set($object);
		// Remove useless properties from the object.
		$object->remove("modules");
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get list of module configuration objects.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getModuleList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.rsyncd.module");
		// Add additional share informations.
		$objectsAssoc = [];
		foreach ($objects as $objectk => &$objectv) {
			// Note, we need to do a stupid workaround here when no users
			// are defined. The problem is caused by the way how the
			// configuration data is represented in XML and expected in JSON.
			// XML: <users></users>
			// JSON: { users: { user: [] } }
			// The workaround takes care that the internal representation
			// will be fixed in the special case when no users are defined.
			if ($objectv->isEmpty("users"))
				$objectv->reset("users");
			// Relocate the users property.
			$objectv->move("users.user", "users");
			// Add the new property 'sharedfoldername'.
			$objectv->add("sharedfoldername", "string", gettext("n/a"));
			// Get the shared folder configuration object.
			$sfObject = $db->get("conf.system.sharedfolder",
			  $objectv->get("sharedfolderref"));
			// Update the 'sharedfoldername' property.
			$objectv->set("sharedfoldername", $sfObject->get("name"));
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a module configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getModule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.rsyncd.module", $params['uuid']);
		// Note, we need to do a stupid workaround here when no users
		// are defined. The problem is caused by the way how the
		// configuration data is represented in XML and expected in JSON.
		// XML: <users></users>
		// JSON: { users: { user: [] } }
		// The workaround takes care that the internal representation
		// will be fixed in the special case when no users are defined.
		if ($object->isEmpty("users"))
			$object->reset("users");
		// Relocate the users property.
		$object->move("users.user", "users");
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Set a module configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setModule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.rsyncd.setmodule");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.rsyncd.module");
		$object->setAssoc($params, TRUE, TRUE);
		$object->set("users.user", $params['users']);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			// Check uniqueness:
			// - Module name
			$db->assertIsUnique($object, "name");
		}
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a module configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteModule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.rsyncd.module", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}
}
