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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class OMVRpcServiceFtp extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "FTP";
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
		$this->registerMethod("getModBanRuleList");
		$this->registerMethod("getModBanRule");
		$this->registerMethod("setModBanRule");
		$this->registerMethod("deleteModBanRule");
		$this->registerMethod("getModTLSSettings");
		$this->registerMethod("setModTLSSettings");
		$this->registerMethod("getStats");
	}

	/**
	 * Get the settings configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ftp");
		// Remove useless properties from the object.
		$object->remove("shares");
		$object->remove("modules");
		return $object->getAssoc();
	}

	/**
	 * Set the settings configuration object.
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
		$this->validateMethodParams($params, "rpc.ftp.setsettings");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ftp");
		$object->setAssoc($params);
		$db->set($object);
		// Remove useless properties from the object.
		$object->remove("shares");
		$object->remove("modules");
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp", $object->getAssoc());
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
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.ftp.share");
		// Add additional share informations.
		$objectsAssoc = array();
		foreach ($objects as $objectk => &$objectv) {
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
		return $db->getAssoc("conf.service.ftp.share", $params['uuid']);
	}

	/**
	 * Set a share config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setShare($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.ftp.setshare");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.share");
		$object->setAssoc($params);
		// Set the configuration object.
		$isNew = $object->isNew();
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $isNew) {
			// Check uniqueness. A shared folder can only be configured once
			// for FTP.
			$db->assertIsUnique($object, "sharedfolderref");
		}
		$db->set($object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify($isNew ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.shares.share", $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
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
		return $this->deleteConfigObjectByUuid("conf.service.ftp.share",
		  $params['uuid'], "org.openmediavault.services.ftp.shares.share");
	}

	/**
	 * Get list of mod_ban rule config objects.
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
	public function getModBanRuleList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the list of configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getAssoc("conf.service.ftp.modban.rule");
		// Filter the result objects.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a ban rule configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getModBanRule($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.ftp.modban.rule", $params['uuid']);
	}

	/**
	 * Set a ban rule configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setModBanRule($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "event":{"type":"string","enum":["AnonRejectPasswords",'.
					'"ClientConnectRate","MaxClientsPerClass",'.
					'"MaxClientsPerHost","MaxClientsPerUser",'.
					'"MaxConnectionsPerHost","MaxHostsPerUser",'.
					'"MaxLoginAttempts","TimeoutIdle","TimeoutNoTransfer"]},
				  "occurrence":{"type":"integer","minimum":1},
				  "timeinterval":{"type":"string","format":"regex",'.
					'"pattern":"\/^[0-9]{2}:[0-9]{2}:[0-9]{2}$\/"},
				  "expire":{"type":"string","format":"regex",'.
					'"pattern":"\/^[0-9]{2}:[0-9]{2}:[0-9]{2}$\/"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  \OMV\Uuid::uuid4() : $params['uuid'],
			"event" => $params['event'],
			"occurrence" => $params['occurrence'],
			"timeinterval" => $params['timeinterval'],
			"expire" => $params['expire']
		);
		// Set the configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//services/ftp/modules/mod_ban",
			  array("rule" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//services/ftp/modules/mod_ban" .
			  "/rule[uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a mod_ban rule config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteModBanRule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$xpath = sprintf("//services/ftp/modules/mod_ban/rule[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule");
	}

	/**
	 * Get the TLS settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getModTLSSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//services/ftp/modules/mod_tls";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['enable'] = boolvalEx($object['enable']);
		$object['required'] = boolvalEx($object['required']);
		$object['nocertrequest'] = boolvalEx($object['nocertrequest']);
		$object['nosessionreuserequired'] = boolvalEx(
		  $object['nosessionreuserequired']);
		$object['useimplicitssl'] = boolvalEx($object['useimplicitssl']);
		return $object;
	}

	/**
	 * Set the TLS settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setModTLSSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "enable":{"type":"boolean"},
				  "required":{"type":"boolean"},
				  "sslcertificateref":{'.
					$GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'},
				  "nocertrequest":{"type":"boolean"},
				  "nosessionreuserequired":{"type":"boolean"},
				  "useimplicitssl":{"type":"boolean"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Set the configuration object.
		$object = array(
			"enable" => array_boolval($params, 'enable'),
			"required" => array_boolval($params, 'required'),
			"sslcertificateref" => $params['sslcertificateref'],
			"nocertrequest" => array_boolval($params, 'nocertrequest'),
			"nosessionreuserequired" => array_boolval($params,
			  'nosessionreuserequired'),
			"useimplicitssl" => array_boolval($params, 'useimplicitssl'),
			"extraoptions" => $params['extraoptions']
		);
		$xpath = "//services/ftp/modules/mod_tls";
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_tls", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get statistics, e.g. who's online.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A string containing the requested statistics.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_EXEC_FAILED
	 */
	public function getStats($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Is service enabled?
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (!boolvalEx($object['enable'])) {
			$stats = gettext("Service disabled");
		} else {
			// Get users currently logged in.
			$stats = sprintf("%s\n", gettext("Currently logged in users:"));
			$cmd = "export LANG=C; ftpwho -v 2>&1";
			if (0 !== $this->exec($cmd, $output))
				throw new \OMV\ExecException($cmd, $output);
			array_shift($output);
			$stats .= implode("\n", $output);
			// Get ban list if module is enabled.
			$xpath = "//services/ftp/modules/mod_ban/rule";
			if(TRUE === $xmlConfig->exists($xpath)) {
				$stats .= sprintf("\n\n%s\n", gettext("Ban statistics:"));
				$cmd = "export LANG=C; ftpdctl -s /var/run/proftpd/proftpd.sock ".
				  "ban info -v -e 2>&1";
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename))
					throw new \OMV\ExecException($cmd, $output);
				foreach ($output as &$outputv) {
					$outputv = preg_replace("/^ftpdctl:/", "", $outputv);
				}
				$stats .= implode("\n", $output);
			}
		}
		return $stats;
	}
}
