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
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceCertificateMgmt extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "CertificateMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("getDetail");
		$this->registerMethod("create");
	}

	/**
	 * Get list of certificate configuration objects.
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
	public function getList($params, $context) {
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
		// Get list of SSL certificate configuration objects.
		$xpath = "//system/certificates/sslcertificate";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional filesystem informations.
		foreach($objects as $objectk => &$objectv) {
			// Parse the certificate
			if($certinfo = openssl_x509_parse($objectv['certificate'])) {
				$objectv['name'] = $certinfo['name'];
				$objectv['validto'] = $certinfo['validTo_time_t'];
			}
			// Remove the certificate and the private key (not needed here).
			unset($objectv['certificate']);
			unset($objectv['privatekey']);
			// Set '_used' flag if filesystem is in use.
			if(TRUE === $xmlConfig->exists("//sslcertificateref[".
			  "contains(.,'{$objectv['uuid']}')]")) {
				$objectv['_used'] = TRUE;
			}
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'], $params['limit'],
		  $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get an certificate configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function get($params, $context) {
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
		$xpath = sprintf("//system/certificates/sslcertificate[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set (add/update) an certificate configuration object
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_MISC_FAILURE
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 * @throw E_CONFIG_SAVE_FAILED
	 */
	function set($params, $context) {
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
				  "certificate":{"type":"string"},
				  "privatekey":{"type":"string"},
				  "comment":{"type":"string"}
			  }
		  }');
		// Validate the certificate.
		if(!($certificate = openssl_x509_read($params['certificate']))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Invalid certificate: %s", openssl_error_string()));
		}
		// Validate the private key.
		if(!($privatekey = openssl_pkey_get_private($params['privatekey']))) {
			openssl_x509_free($certificate);
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Invalid private key: %s", openssl_error_string()));
		}
		// Check if the private key corresponds to the certificate.
		if(!openssl_x509_check_private_key($certificate, $privatekey)) {
			openssl_x509_free($certificate);
			openssl_pkey_free($privatekey);
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Private key does not correspond to the certificate: %s",
			  openssl_error_string()));
		} 
		openssl_x509_free($certificate);
		openssl_pkey_free($privatekey); 
		// Prepare configuration data.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"certificate" => $params['certificate'],
			"privatekey" => $params['privatekey'],
			"comment" => $params['comment']
		);
		// Set the configuration data.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//system/certificates",
			  array("sslcertificate" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//system/certificates/".
			  "sslcertificate[uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save the configuration.
		if(FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getLastError());
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.certificates.sslcertificate", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a certificate config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 * @throw E_CONFIG_OBJECT_INUSE
	 */
	public function delete($params, $context) {
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
		// Check if the certificate is used somewhere.
		$xpath = sprintf("//sslcertificateref[contains(.,'%s')]",
		  $params['uuid']);
		if(TRUE === $xmlConfig->exists($xpath)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INUSE);
		}
		// Delete the configuration object.
		$xpath = sprintf("//system/certificates/sslcertificate[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.certificates.sslcertificate");
	}

	/**
	 * Get detail about a certificate.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return A string containing the certificate details.
	 * @throw E_CONFIG_DIRTY
	 * @throw E_EXEC_FAILED
	 */
	public function getDetail($params, $context) {
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
		// Check if the module is marked as dirty. This is an indication
		// that the certificate has not been created or updated until now.
		if ($this->isModuleDirty("certificatemgmt"))
			throw new OMVException(OMVErrorMsg::E_CONFIG_DIRTY);
		// Get certificate details.
		$cmd = sprintf("openssl x509 -text -noout -in ".
		  "%s/certs/%s%s.crt 2>&1", $GLOBALS['OMV_CERTIFICATE_DIR'],
		  $GLOBALS['OMV_CERTIFICATE_PREFIX'], $params['uuid']);
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		return implode("\n", $output);
	}

	/**
	 * Create a certificate.
	 * @param params An array containing the following fields:
	 *   \em size The size.
	 *   \em days The number of days the certificate is valid for.
	 *   \em c Country
	 *   \em st State or province
	 *   \em l Locality/city
	 *   \em o Organization name
	 *   \em ou Organization unit name
	 *   \em cn Common name
	 *   \em email Email address
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_EXEC_FAILED
	 */
	public function create($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "size":{"type":"integer","enum":[512,1024,2048,4096]},
				  "days":{"type":"integer"},
				  "c":{"type":"string","maxLength":2},
				  "st":{"type":"string"},
				  "l":{"type":"string"},
				  "o":{"type":"string"},
				  "ou":{"type":"string"},
				  "cn":{"type":"string","minLength":1},
				  "email":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'}
			  }
		  }');
		// Generate the certificate subject string.
		$rdnfields = array("c", "st", "l", "o", "ou", "cn");
		$subject = "";
		foreach($rdnfields as $rdnfieldk => $rdnfieldv) {
			if(empty($params[$rdnfieldv]))
				continue;
			// Escape RDN attribute value, see
			// http://www-03.ibm.com/systems/i/software/ldap/underdn.html
			// http://msdn.microsoft.com/en-us/library/windows/desktop/aa366101(v=vs.85).aspx
			$replace = array();
			foreach(array(',','+','<','>',';','\r','=','#','/') as $v) {
				$replace[$v] = sprintf("\\%s", strtoupper(str_pad(dechex(
				  ord($v)), 2, "0")));
			}
			$params[$rdnfieldv] = strtr($params[$rdnfieldv], $replace);
			$params[$rdnfieldv] = addcslashes($params[$rdnfieldv], '"\\');
			// Append to subject.
			switch($rdnfieldv) {
			case "cn":
				// Append the email address.
				if(!empty($params['email'])) {
					$params[$rdnfieldv] = sprintf("%s/emailAddress=%s",
					  $params[$rdnfieldv], $params['email']);
				}
			default:
				$subject .= sprintf("/%s=%s", strtoupper($rdnfieldv),
				  $params[$rdnfieldv]);
			}
		}
		// Create the requested certificate.
		// http://www.zytrax.com/tech/survival/ssl.html
		$keyfile = tempnam(sys_get_temp_dir(), "cert");
		$crtfile = tempnam(sys_get_temp_dir(), "cert");
		$cmd = sprintf("export LANG=C; openssl req -x509 -nodes -days %d ".
		  "-sha256 -batch -newkey rsa:%d -keyout %s -out %s",
		  $params['days'], $params['size'], $keyfile, $crtfile);
		if(!empty($subject)) {
			$cmd .= sprintf(" -subj '%s'", $subject);
		}
		$cmd .= " 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			// Unlink the temporary files.
			unlink($keyfile);
			unlink($crtfile);
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Read certificate and key content, then unlink the temporary files.
		$keydata = file_get_contents($keyfile);
		$crtdata = file_get_contents($crtfile);
		unlink($keyfile);
		unlink($crtfile);
		// Finally import generated certificate.
		return $this->callMethod("set", array(
			  "uuid" => $GLOBALS['OMV_UUID_UNDEFINED'],
			  "certificate" => $crtdata,
			  "privatekey" => $keydata,
			  "comment" => !empty($subject) ? $subject : gettext("n/a")
		  ), $context);
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceCertificateMgmt());
?>
