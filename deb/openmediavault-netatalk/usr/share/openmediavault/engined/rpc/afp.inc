<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
class OMVRpcServiceAFP extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "AFP";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getShareList");
		$this->registerMethod("getShare");
		$this->registerMethod("setShare");
		$this->registerMethod("deleteShare");
	}

	/**
	 * Get settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.afp");
		// Remove useless properties from the object.
		$object->remove("shares");
		return $object->getAssoc();
	}

	/**
	 * Set settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.afp.setsettings");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.afp");
		$object->setAssoc($params);
		$db->set($object);
		// Remove useless properties from the object.
		$object->remove("shares");
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.service.afp", $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get list of share configuration objects.
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
	public function getShareList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.afp.share");
		// Add additional share information.
		$objectsAssoc = [];
		foreach ($objects as $objectk => &$objectv) {
			// Add the new property 'sharedfoldername'.
			$objectv->add("sharedfoldername", "string", gettext("n/a"));
			// Get the shared folder configuration object.
			$sfObject = $db->get("conf.system.sharedfolder",
			  $objectv->get("sharedfolderref"));
			// Update the 'sharedfoldername' property.
			$objectv->set("sharedfoldername", $sfObject->get("name"));
			// Relocate the options.
			$options = $objectv->get("options");
			foreach ($options as $optionk => $optionv) {
				$objectv->add($optionk, "boolean");
				$objectv->set($optionk, $optionv);
			}
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a share config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->getAssoc("conf.service.afp.share", $params['uuid']);
		// Relocate the options.
		$options = $object->get("options");
		foreach ($options as $optionk => $optionv) {
			$object->add($optionk, "boolean");
			$object->set($optionk, $optionv);
		}
		return $object->getAssoc();
	}

	/**
	 * Set a share config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.afp.setshare");


// ToDo...


		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  \OMV\Uuid::uuid4() : $params['uuid'],
			"sharedfolderref" => $params['sharedfolderref'],
			"name" => $params['name'],
			"comment" => $params['comment'],
			"password" => $params['password'],
			"casefold" => $params['casefold'],
			"volsizelimit" => $params['volsizelimit'],
			"allowguest" => array_boolval($params, 'allowguest'),
			"guestrw" => array_boolval($params, 'guestrw'),
			"options" => array(
				"ro" => array_boolval($params, 'ro'),
				"upriv" => array_boolval($params, 'upriv'),
				"usedots" => array_boolval($params, 'usedots'),
				"invisibledots" => array_boolval($params, 'invisibledots'),
				"tm" => array_boolval($params, 'tm'),
				"mswindows" => array_boolval($params, 'mswindows')
			),
			"extraoptions" => $params['extraoptions']
		);
		// Set the configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//services/afp/shares",
			  array("share" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//services/afp/shares/share[" .
			  "uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.service.afp.share", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a share configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		return $this->deleteConfigObjectByUuid("conf.service.afp.share",
		  $params['uuid'], "org.openmediavault.conf.service.afp.share");
	}
}
