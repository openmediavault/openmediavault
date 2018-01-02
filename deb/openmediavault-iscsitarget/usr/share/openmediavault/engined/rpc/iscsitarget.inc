<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2018 Volker Theile
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
class OMVRpcServiceIscsiTarget extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "iSCSITarget";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getTargetList");
		$this->registerMethod("getTarget");
		$this->registerMethod("setTarget");
		$this->registerMethod("deleteTarget");
	}

	/**
	 * Get the configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.iscsitarget");
		// Note, we need to do a stupid workaround here when no users
		// are defined. The problem is caused by the way how the
		// configuration data is represented in XML and expected in JSON.
		// XML: <authentication></authentication>
		// JSON: { authentication: { user: [] } }
		// The workaround takes care that the internal representation
		// will be fixed in the special case when no users are defined.
		if ($object->isEmpty("authentication"))
			$object->reset("authentication");
		// Relocate the authentication property.
		$object->move("authentication.user", "authentication");
		// Remove useless properties from the object.
		$object->remove("targets");
		return $object->getAssoc();
	}

	/**
	 * Set the configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.iscsitarget.setsettings");
		// Update the authentication users uuid's (only necessary if the
		// user is new).
		if (!empty($params['authentication'])) {
			foreach ($params['authentication'] as $userk => &$userv) {
				if ($userv['uuid'] !== \OMV\Environment::get(
				  "OMV_CONFIGOBJECT_NEW_UUID"))
					continue;
				$userv['uuid'] = \OMV\Uuid::uuid4();
			}
		}
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.iscsitarget");
		$object->setAssoc($params, TRUE, TRUE);
		$object->set("authentication", [
			"user" => $params['authentication']
		]);
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get list of target configuration objects.
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
	public function getTargetList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.iscsitarget.target");
		// Add additional information.
		$objectsAssoc = [];
		$rdomainname = strtolower(implode(".", array_reverse(explode(".",
		  \OMV\System\Net\Dns::getFqdn()))));
		foreach ($objects as $objectk => &$objectv) {
			$iqn = sprintf("iqn.%s.%s", $objectv->get("activation"),
			  $rdomainname);
			if (FALSE === $objectv->isEmpty("identifier"))
				$iqn .= sprintf(":%s", $objectv->get("identifier"));
			$objectv->add("iqn", "string", $iqn);
			// Note, we need to do a stupid workaround here when no luns
			// are defined. The problem is caused by the way how the
			// configuration data is represented in XML and expected in JSON.
			// XML: <luns></luns>
			// JSON: { luns: { lun: [] } }
			// The workaround takes care that the internal representation
			// will be fixed in the special case when no luns are defined.
			if ($objectv->isEmpty("authentication"))
				$objectv->reset("authentication");
			if ($objectv->isEmpty("luns"))
				$objectv->reset("luns");
			// Relocate the properties.
			$objectv->move("authentication.user", "authentication");
			$objectv->move("luns.lun", "luns");
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a target configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getTarget($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.iscsitarget.target", $params['uuid']);
		// Note, we need to do a stupid workaround here when no luns
		// are defined. The problem is caused by the way how the
		// configuration data is represented in XML and expected in JSON.
		// XML: <luns></luns>
		// JSON: { luns: { lun: [] } }
		// The workaround takes care that the internal representation
		// will be fixed in the special case when no luns are defined.
		if ($object->isEmpty("authentication"))
			$object->reset("authentication");
		if ($object->isEmpty("luns"))
			$object->reset("luns");
		// Relocate the properties.
		$object->move("authentication.user", "authentication");
		$object->move("luns.lun", "luns");
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Set a target configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setTarget($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.iscsitarget.settarget");
		// Update the LUNs uuid's (only necessary if the LUN is new).
		if (!empty($params['luns'])) {
			foreach ($params['luns'] as $lunk => &$lunv) {
				if ($lunv['uuid'] !== \OMV\Environment::get(
				  "OMV_CONFIGOBJECT_NEW_UUID"))
					continue;
				$lunv['uuid'] = \OMV\Uuid::uuid4();
			}
		}
		// Update the authentication users uuid's (only necessary if the
		// user is new).
		if (!empty($params['authentication'])) {
			foreach ($params['authentication'] as $userk => &$userv) {
				if ($userv['uuid'] !== \OMV\Environment::get(
				  "OMV_CONFIGOBJECT_NEW_UUID"))
					continue;
				$userv['uuid'] = \OMV\Uuid::uuid4();
			}
		}
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject(
		  "conf.service.iscsitarget.target");
		$object->setAssoc(array_merge($params, [
			"authentication" => [
				"user" => $params['authentication']
			],
			"luns" => [
				"lun" => $params['luns']
			]
		]));
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			$object->set("activation", date("Y-m"));
		}
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a target configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteTarget($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.iscsitarget.target", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}
}
