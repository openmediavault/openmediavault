<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceFtp extends OMVRpcServiceAbstract {
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
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['enable'] = boolvalEx($object['enable']);
		$object['port'] = intval($object['port']);
		$object['maxclients'] = intval($object['maxclients']);
		$object['maxconnectionsperhost'] = intval(
		  $object['maxconnectionsperhost']);
		$object['maxloginattempts'] = intval($object['maxloginattempts']);
		$object['timeoutidle'] = intval($object['timeoutidle']);
		$object['allowforeignaddress'] = boolvalEx(
		  $object['allowforeignaddress']);
		$object['allowrestart'] = boolvalEx($object['allowrestart']);
		$object['identlookups'] = boolvalEx($object['identlookups']);
		$object['usereversedns'] = boolvalEx($object['usereversedns']);
		$object['rootlogin'] = boolvalEx($object['rootlogin']);
		$object['usepassiveports'] = boolvalEx($object['usepassiveports']);
		$object['minpassiveports'] = intval($object['minpassiveports']);
		$object['maxpassiveports'] = intval($object['maxpassiveports']);
		$object['limittransferrate'] = boolvalEx($object['limittransferrate']);
		$object['maxuptransferrate'] = intval($object['maxuptransferrate']);
		$object['maxdowntransferrate'] = intval($object['maxdowntransferrate']);
		$object['anonymous'] = boolvalEx($object['anonymous']);
		$object['requirevalidshell'] = boolvalEx($object['requirevalidshell']);
		$object['dynmasqrefresh'] = intval($object['dynmasqrefresh']);
		$object['transferlog'] = boolvalEx($object['transferlog']);
		// Remove useless fields from the result object.
		unset($object['shares']);
		unset($object['modules']);
		return $object;
	}

	/**
	 * Set the settings configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setSettings($params, $context) {
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
				  "port":{"type":"integer","minimum":1,"maximum":65535},
				  "maxclients":{"type":"integer","minimum":0},
				  "maxconnectionsperhost":{"type":"integer","minimum":0},
				  "maxloginattempts":{"type":"integer","minimum":0},
				  "timeoutidle":{"type":"integer","minimum":0},
				  "displaylogin":{"type":"string"},
				  "allowforeignaddress":{"type":"boolean"},
				  "allowrestart":{"type":"boolean"},
				  "identlookups":{"type":"boolean"},
				  "usereversedns":{"type":"boolean"},
				  "rootlogin":{"type":"boolean"},
				  "masqueradeaddress":{"type":"string"},
				  "dynmasqrefresh":{"type":"integer","minimum":0},
				  "usepassiveports":{"type":"boolean"},
				  "minpassiveports":{"type":"integer","minimum":1025,'.
					'"maximum":65535},
				  "maxpassiveports":{"type":"integer","minimum":1025,'.
					'"maximum":65535},
				  "limittransferrate":{"type":"boolean"},
				  "maxuptransferrate":{"type":"integer","minimum":0},
				  "maxdowntransferrate":{"type":"integer","minimum":0},
				  "anonymous":{"type":"boolean"},
				  "requirevalidshell":{"type":"boolean"},
				  "transferlog":{"type":"boolean"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Get the existing configuration object.
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the configuration object.
		$object = array_merge($object, array(
			  "enable" => array_boolval($params, 'enable'),
			  "port" => $params['port'],
			  "maxclients" => $params['maxclients'],
			  "maxconnectionsperhost" => $params['maxconnectionsperhost'],
			  "maxloginattempts" => $params['maxloginattempts'],
			  "timeoutidle" => $params['timeoutidle'],
			  "displaylogin" => $params['displaylogin'],
			  "allowforeignaddress" => array_boolval($params,
				'allowforeignaddress'),
			  "allowrestart" => array_boolval($params, 'allowrestart'),
			  "identlookups" => array_boolval($params, 'identlookups'),
			  "usereversedns" => array_boolval($params, 'usereversedns'),
			  "rootlogin" => array_boolval($params, 'rootlogin'),
			  "masqueradeaddress" => $params['masqueradeaddress'],
			  "dynmasqrefresh" => $params['dynmasqrefresh'],
			  "usepassiveports" => array_boolval($params, 'usepassiveports'),
			  "minpassiveports" => $params['minpassiveports'],
			  "maxpassiveports" => $params['maxpassiveports'],
			  "limittransferrate" => array_boolval($params,
				'limittransferrate'),
			  "maxuptransferrate" => $params['maxuptransferrate'],
			  "maxdowntransferrate" => $params['maxdowntransferrate'],
			  "anonymous" => array_boolval($params, 'anonymous'),
			  "requirevalidshell" => array_boolval($params,
				'requirevalidshell'),
			  "transferlog" => array_boolval($params, 'transferlog'),
			  "extraoptions" => $params['extraoptions']
		  ));
		if(FALSE === $xmlConfig->replace("//services/ftp", $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp", $object);
		// Return the configuration object.
		return $object;
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
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getShareList($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = "//services/ftp/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations.
		foreach($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object.
			$sfObject = $xmlConfig->get("//system/shares/sharedfolder[".
			  "uuid='{$objectv['sharedfolderref']}']");
			if(!is_null($sfObject) && !empty($sfObject)) {
				$objectv['sharedfoldername'] = $sfObject['name'];
			}
		}
		// Filter the result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a share config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getShare($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//services/ftp/shares/share[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
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
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "comment":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"sharedfolderref" => $params['sharedfolderref'],
			"comment" => $params['comment']
		);
		// Set the configuration object.
		$success = FALSE;
		if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness. A shared folder can only be configured once
			// for FTP.
			if (TRUE === $xmlConfig->exists("//services/ftp/shares/share[" .
			  "sharedfolderref='{$params['sharedfolderref']}']")) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  gettext("The shared folder is already used by another share."));
			}
			// Append the configuration object.
			$success = $xmlConfig->set("//services/ftp/shares",
			  array("share" => $object));
		} else {
			// Update the existing configuration object.
			$success = $xmlConfig->replace("//services/ftp/shares/share[" .
			  "uuid='{$params['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.shares.share", $object);
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
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Delete the configuration object.
		$xpath = sprintf("//services/ftp/shares/share[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.ftp.shares.share");
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
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getModBanRuleList($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Get the list of configuration objects.
		$xpath = "//services/ftp/modules/mod_ban/rule";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result objects.
		foreach($objects as $objectk => &$objectv) {
			$objectv['occurrence'] = intval($objectv['occurrence']);
		}
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
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//services/ftp/modules/mod_ban/rule[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['occurrence'] = intval($object['occurrence']);
		return $object;
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
			  OMVUtil::uuid() : $params['uuid'],
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
		$dispatcher = &OMVNotifyDispatcher::getInstance();
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
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
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
		$dispatcher = &OMVNotifyDispatcher::getInstance();
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
			$cmd = "ftpwho -v 2>&1";
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			array_shift($output);
			$stats .= implode("\n", $output);
			// Get ban list if module is enabled.
			$xpath = "//services/ftp/modules/mod_ban/rule";
			if(TRUE === $xmlConfig->exists($xpath)) {
				$stats .= sprintf("\n\n%s\n", gettext("Ban statistics:"));
				$cmd = "ftpdctl -s /var/run/proftpd/proftpd.sock ".
				  "ban info -v -e 2>&1";
				if(0 !== $this->exec($cmd, $output)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
				foreach($output as &$outputv) {
					$outputv = preg_replace("/^ftpdctl:/", "", $outputv);
				}
				$stats .= implode("\n", $output);
			}
		}
		return $stats;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceFtp());
?>
