<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class FTPRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
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
					"extraoptions":{"type":"string"},
					"dnssdenable":{"type":"boolean"},
					"dnssdname":{"type":"string"}
				}
			}'),
			"getShareList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"mntentref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"comment":{"type":"string"}
				}
			}'),
			"deleteShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"getModBanRuleList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getModBanRule" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setModBanRule" => array('{
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
			}'),
			"deleteModBanRule" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setModTLSSettings" => array('{
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
			}')
		);
	}

	/**
	 * Get the FTP configuration object.
	 */
	public function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['port'] = intval($object['port']);
		$object['maxclients'] = intval($object['maxclients']);
		$object['maxconnectionsperhost'] = intval(
		  $object['maxconnectionsperhost']);
		$object['maxloginattempts'] = intval($object['maxloginattempts']);
		$object['timeoutidle'] = intval($object['timeoutidle']);
		$object['allowforeignaddress'] = boolval(
		  $object['allowforeignaddress']);
		$object['allowrestart'] = boolval($object['allowrestart']);
		$object['identlookups'] = boolval($object['identlookups']);
		$object['usereversedns'] = boolval($object['usereversedns']);
		$object['rootlogin'] = boolval($object['rootlogin']);
		$object['usepassiveports'] = boolval($object['usepassiveports']);
		$object['minpassiveports'] = intval($object['minpassiveports']);
		$object['maxpassiveports'] = intval($object['maxpassiveports']);
		$object['limittransferrate'] = boolval($object['limittransferrate']);
		$object['maxuptransferrate'] = intval($object['maxuptransferrate']);
		$object['maxdowntransferrate'] = intval($object['maxdowntransferrate']);
		$object['anonymous'] = boolval($object['anonymous']);
		$object['requirevalidshell'] = boolval($object['requirevalidshell']);
		$object['dnssdenable'] = boolval($object['dnssd']['enable']);
		$object['dnssdname'] = $object['dnssd']['name'];
		$object['dynmasqrefresh'] = intval($object['dynmasqrefresh']);
		// Remove useless data
		unset($object['shares']);
		unset($object['modules']);
		unset($object['dnssd']);
		return $object;
	}

	public function setSettings($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get existing configuration object
		$xpath = "//services/ftp";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"port" => $data['port'],
			"maxclients" => $data['maxclients'],
			"maxconnectionsperhost" => $data['maxconnectionsperhost'],
			"maxloginattempts" => $data['maxloginattempts'],
			"timeoutidle" => $data['timeoutidle'],
			"displaylogin" => $data['displaylogin'],
			"allowforeignaddress" => array_boolval($data,
			  'allowforeignaddress'),
			"allowrestart" => array_boolval($data, 'allowrestart'),
			"identlookups" => array_boolval($data, 'identlookups'),
			"usereversedns" => array_boolval($data, 'usereversedns'),
			"rootlogin" => array_boolval($data, 'rootlogin'),
			"masqueradeaddress" => $data['masqueradeaddress'],
			"dynmasqrefresh" => $data['dynmasqrefresh'],
			"usepassiveports" => array_boolval($data, 'usepassiveports'),
			"minpassiveports" => $data['minpassiveports'],
			"maxpassiveports" => $data['maxpassiveports'],
			"limittransferrate" => array_boolval($data, 'limittransferrate'),
			"maxuptransferrate" => $data['maxuptransferrate'],
			"maxdowntransferrate" => $data['maxdowntransferrate'],
			"anonymous" => array_boolval($data, 'anonymous'),
			"requirevalidshell" => array_boolval($data, 'requirevalidshell'),
			"extraoptions" => $data['extraoptions'],
			"shares" => isset($oldObject['shares']) ?
			  $oldObject['shares'] : array(),
			"modules" => isset($oldObject['modules']) ?
			  $oldObject['modules'] : array(),
			"dnssd" => array(
				"enable" => array_boolval($data, 'dnssdenable'),
				"name" => $data['dnssdname']
			)
		);
		if (FALSE === $xmlConfig->replace("//services/ftp", $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp", $object);
	}

	/**
	 * Get list of share configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getShareList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = "//services/ftp/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations
		foreach ($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[".
			  "uuid='{$objectv['sharedfolderref']}']");
			if (!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a share config object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	public function getShare($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = sprintf("//services/ftp/shares/share[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set a share config object.
	 */
	public function setShare($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"sharedfolderref" => $data['sharedfolderref'],
			"mntentref" => $data['mntentref'],
			"comment" => $data['comment']
		);
		// Set the configuration object.
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness. A shared folder can only be configured once
			// for FTP.
			if (TRUE === $xmlConfig->exists("//services/ftp/shares/share[" .
			  "sharedfolderref='{$data['sharedfolderref']}']")) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  gettext("The shared folder is already used by another share"));
			}
			// Get the shared folder object.
			$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
			  $data['sharedfolderref']);
			$sfobject = $xmlConfig->get($xpath);
			if (is_null($sfobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Get the absolute shared folder path.
			$sfpath = OMVRpc::exec("ShareMgmt", "getPath", array(
			  "uuid" => $data['sharedfolderref']));
			// Prepare fstab mount point entry configuration object to bind
			// the shared folder directory into the FTP users home directory.
			$meobject = array(
				"uuid" => OMVUtil::uuid(),
				"fsname" => $sfpath,
				"dir" => sprintf("/home/ftp/%s", $sfobject['name']),
				"type" => "none",
				"opts" => "bind",
				"freq" => 0,
				"passno" => 0
			);
			// Modify the configuration object.
			$object['mntentref'] = $meobject['uuid'];
			// Append the configuration object.
			$success = $xmlConfig->set("//services/ftp/shares",
			  array("share" => $object));
			// Append assosiated fstab mount point entry configuration object.
			if (FALSE === $xmlConfig->set("//system/fstab", array(
			  "mntent" => $meobject))) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
			}
		} else {
			// Update the existing configuration object.
			$success = $xmlConfig->replace("//services/ftp/shares/share[" .
			  "uuid='{$data['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$dispatcher->notify(OMV_NOTIFY_CREATE,
			  "org.openmediavault.system.fstab.mntent", $meobject);
		}
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.shares.share", $object);
	}

	/**
	 * Delete a share configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function deleteShare($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Delete the configuration object.
		$xpath = sprintf("//services/ftp/shares/share[uuid='%s']",
		  $data['uuid']);
		$object = $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.ftp.shares.share");
		// Delete the associated fstab mount point entry.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.fstab.mntent");
	}

	/**
	 * Get list of mod_ban rule config objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getModBanRuleList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get list of configuration objects
		$xpath = "//services/ftp/modules/mod_ban/rule";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		foreach ($objects as $objectk => &$objectv) {
			$objectv['occurrence'] = intval($objectv['occurrence']);
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a mod_ban rule config object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	public function getModBanRule($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = sprintf("//services/ftp/modules/mod_ban/rule[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['occurrence'] = intval($object['occurrence']);
		return $object;
	}

	/**
	 * Set a mod_ban rule config object.
	 */
	public function setModBanRule($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"event" => $data['event'],
			"occurrence" => $data['occurrence'],
			"timeinterval" => $data['timeinterval'],
			"expire" => $data['expire']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//services/ftp/modules/mod_ban",
			  array("rule" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//services/ftp/modules/mod_ban" .
			  "/rule[uuid='{$data['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule", $object);
	}

	/**
	 * Delete a mod_ban rule config object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function deleteModBanRule($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Delete the configuration object.
		$xpath = sprintf("//services/ftp/modules/mod_ban/rule[uuid='%s']",
		  $data['uuid']);
		$this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule");
	}

	public function getModTLSSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/ftp/modules/mod_tls";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['required'] = boolval($object['required']);
		$object['nocertrequest'] = boolval($object['nocertrequest']);
		$object['nosessionreuserequired'] = boolval(
		  $object['nosessionreuserequired']);
		$object['useimplicitssl'] = boolval($object['useimplicitssl']);
		return $object;
	}

	public function setModTLSSettings($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Set configuration object
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"required" => array_boolval($data, 'required'),
			"sslcertificateref" => $data['sslcertificateref'],
			"nocertrequest" => array_boolval($data, 'nocertrequest'),
			"nosessionreuserequired" => array_boolval($data,
			  'nosessionreuserequired'),
			"useimplicitssl" => array_boolval($data, 'useimplicitssl'),
			"extraoptions" => $data['extraoptions']
		);
		$xpath = "//services/ftp/modules/mod_tls";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_tls", $object);
	}

	/**
	 * Get statistics, e.g. who's online.
	 */
	public function getStats() {
		global $xmlConfig;
		// Is service enabled?
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (!array_boolval($object, 'enable')) {
			$stats = gettext("Service disabled");
		} else {
			// Get users currently logged in
			$stats = sprintf("%s\n", gettext("Currently logged in users:"));
			$cmd = "sudo ftpwho -v 2>&1";
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			array_shift($output);
			$stats .= implode("\n", $output);

			// Get ban list if module is enabled
			$xpath = "//services/ftp/modules/mod_ban/rule";
			if (TRUE === $xmlConfig->exists($xpath)) {
				$stats .= sprintf("\n\n%s\n", gettext("Ban statistics:"));
				unset($output);
				$cmd = "sudo ftpdctl -s /var/run/proftpd/proftpd.sock ban info -v -e 2>&1";
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
				foreach ($output as &$outputv) {
					$outputv = preg_replace("/^ftpdctl:/", "", $outputv);
				}
				$stats .= implode("\n", $output);
			}
		}
		return $stats;
	}
}
?>
