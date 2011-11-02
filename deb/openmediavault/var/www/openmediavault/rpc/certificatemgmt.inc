<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class CertificateMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"get" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"set" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"certificate":{"type":"string"},
					"privatekey":{"type":"string"},
					"comment":{"type":"string"}
				}
			}'),
			"delete" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"getDetail" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			)
		);
	}

	/**
	 * Get list of certificate config objects.
	 * @return A list of certificates.
	 */
	public function getList($start, $count, $sortField, $sortDir) {
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
		// Get list of SSL certificate configuration objects
		$xpath = "//system/certificates/sslcertificate";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional filesystem informations
		foreach ($objects as $objectk => &$objectv) {
			// Parse the certificate
			if ($certinfo = openssl_x509_parse($objectv['certificate'])) {
				$objectv['name'] = $certinfo['name'];
				$objectv['validto'] = $certinfo['validTo_time_t'];
			}
			// Remove the certificate and the private key (not needed here)
			unset($objectv['certificate']);
			unset($objectv['privatekey']);
			// Set '_used' flag if filesystem is in usage
			if (TRUE === $xmlConfig->exists("//sslcertificateref[" .
			  "contains(.,'{$objectv['uuid']}')]")) {
				$objectv['_used'] = true;
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get an certificate configuration object
	 */
	function get($uuid) {
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
		$xpath = "//system/certificates/sslcertificate[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set (add/update) an certificate configuration object
	 */
	function set($data) {
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
		// Validate the certificate
		if (!($certificate = openssl_x509_read($data['certificate']))) {
			throw new ErrorMsgException(ErrorMsg::E_MISC_FAILURE,
			  "Invalid certificate");
		}
		// Validate the private key
		if (!($privatekey = openssl_pkey_get_private($data['privatekey']))) {
			openssl_x509_free($certificate);
			throw new ErrorMsgException(ErrorMsg::E_MISC_FAILURE,
			  "Invalid private key");
		}
		// Check if the private key corresponds to the certificate
		if (!openssl_x509_check_private_key($certificate, $privatekey)) {
			openssl_x509_free($certificate);
			openssl_pkey_free($privatekey);
			throw new ErrorMsgException(ErrorMsg::E_MISC_FAILURE,
			  "Private key does not correspond to the certificate");
		} 
		openssl_x509_free($certificate);
		openssl_pkey_free($privatekey); 
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"certificate" => $data['certificate'],
			"privatekey" => $data['privatekey'],
			"comment" => $data['comment']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//system/certificates",
			  array("sslcertificate" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//system/certificates/" .
			  "sslcertificate[uuid='{$data['uuid']}']", $object);
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
		  "org.openmediavault.system.certificates.sslcertificate", $object);
	}

	/**
	 * Delete a certificate config object.
	 */
	public function delete($uuid) {
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
		// Check if the certificate is used somewhere
		if (TRUE === $xmlConfig->exists("//sslcertificateref[" .
		  "contains(.,'{$uuid}')]")) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INUSE);
		}
		// Get the object
		$xpath = "//system/certificates/sslcertificate[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.certificates.sslcertificate", $object);
		// Delete configuration object
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.certificates.sslcertificate", $object);
	}

	/**
	 * Get detail about a certificate
	 */
	public function getDetail($uuid) {
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
		// Get certificate details
		$cmd = sprintf("sudo openssl x509 -text -noout -in " .
		  "%s/certs/%s.crt 2>&1", $GLOBALS['OMV_CERTIFICATE_DIR'], $uuid);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		return implode("\n", $output);
	}
}
?>
