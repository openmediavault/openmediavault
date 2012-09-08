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
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class CertificateMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"get" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"set" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"certificate":{"type":"string"},
					"privatekey":{"type":"string"},
					"comment":{"type":"string"}
				}
			}'),
			"delete" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"getDetail" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"create" => array('{
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
			}')
		);
	}

	/**
	 * Get list of certificate configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getList($data) {
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
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get an certificate configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	function get($data) {
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
		$xpath = sprintf("//system/certificates/sslcertificate[uuid='%s']",
		  $data['uuid']);
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
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Invalid certificate: %s", openssl_error_string()));
		}
		// Validate the private key
		if (!($privatekey = openssl_pkey_get_private($data['privatekey']))) {
			openssl_x509_free($certificate);
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Invalid private key: %s", openssl_error_string()));
		}
		// Check if the private key corresponds to the certificate
		if (!openssl_x509_check_private_key($certificate, $privatekey)) {
			openssl_x509_free($certificate);
			openssl_pkey_free($privatekey);
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Private key does not correspond to the certificate: %s",
			  openssl_error_string()));
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
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function delete($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the certificate is used somewhere.
		$xpath = sprintf("//sslcertificateref[contains(.,'%s')]",
		  $data['uuid']);
		if (TRUE === $xmlConfig->exists($xpath)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INUSE);
		}
		// Delete the configuration object.
		$xpath = sprintf("//system/certificates/sslcertificate[uuid='%s']",
		  $data['uuid']);
		$this->deleteConfigObject($xpath,
		  "org.openmediavault.system.certificates.sslcertificate");
	}

	/**
	 * Get detail about a certificate.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return A string containing the certificate details.
	 */
	public function getDetail($data) {
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
		  "%s/certs/%s.crt 2>&1", $GLOBALS['OMV_CERTIFICATE_DIR'],
		  $data['uuid']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		return implode("\n", $output);
	}

	/**
	 * Create a certificate.
	 * @param data An array containing the following fields:
	 *   \em size The size.
	 *   \em days The number of days the certificate is valid for.
	 *   \em c Country
	 *   \em st State or province
	 *   \em l Locality/city
	 *   \em o Organization name
	 *   \em ou Organization unit name
	 *   \em cn Common name
	 *   \em email Email address
	 * @return None
	 */
	public function create($data) {
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
		// Generate the certificate subject string
		$rdnfields = array("c", "st", "l", "o", "ou", "cn");
		$subject = "";
		foreach($rdnfields as $rdnfieldk => $rdnfieldv) {
			if (empty($data[$rdnfieldv]))
				continue;
			// Escape RDN attribute value, see
			// http://www-03.ibm.com/systems/i/software/ldap/underdn.html
			// http://msdn.microsoft.com/en-us/library/windows/desktop/aa366101(v=vs.85).aspx
			$replace = array();
			foreach (array(',','+','<','>',';','\r','=','#','/') as $v) {
				$replace[$v] = sprintf("\\%s", strtoupper(str_pad(dechex(
				  ord($v)), 2, "0")));
			}
			$data[$rdnfieldv] = strtr($data[$rdnfieldv], $replace);
			$data[$rdnfieldv] = addcslashes($data[$rdnfieldv], '"\\');
			// Append to subject
			switch ($rdnfieldv) {
			case "cn":
				// Append the email address
				if (!empty($data['email'])) {
					$data[$rdnfieldv] = sprintf("%s/emailAddress=%s",
					  $data[$rdnfieldv], $data['email']);
				}
			default:
				$subject .= sprintf("/%s=%s", strtoupper($rdnfieldv),
				  $data[$rdnfieldv]);
			}
		}
		// Create the requested certificate
		// http://www.zytrax.com/tech/survival/ssl.html
		$keyfile = tempnam(sys_get_temp_dir(), "");
		$crtfile = tempnam(sys_get_temp_dir(), "");
		$cmd = sprintf("export LANG=C; openssl req -x509 -nodes -days %d ".
		  "-batch -newkey rsa:%d -keyout %s -out %s",
		  $data['days'], $data['size'], $keyfile, $crtfile);
		if (!empty($subject)) {
			$cmd .= sprintf(" -subj '%s'", $subject);
		}
		$cmd .= " 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			unlink($keyfile);
			unlink($crtfile);
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Read certificate and key content, then unlink the temporary files
		$keydata = file_get_contents($keyfile);
		$crtdata = file_get_contents($crtfile);
		unlink($keyfile);
		unlink($crtfile);
		// Finally import generated certificate
		OMVRpc::exec("CertificateMgmt", "set", array(
		  "uuid" => $GLOBALS['OMV_UUID_UNDEFINED'],
		  "certificate" => $crtdata,
		  "privatekey" => $keydata,
		  "comment" => !empty($subject) ? $subject : gettext("n/a")));
	}
}
?>



<?php 
function ldap_escape($str, $for_dn = false) 
{ 
    
    // see: 
    // RFC2254 
    // http://msdn.microsoft.com/en-us/library/ms675768(VS.85).aspx 
http://msdn.microsoft.com/en-us/library/windows/desktop/aa366101(v=vs.85).aspx
    // http://www-03.ibm.com/systems/i/software/ldap/underdn.html        
        
    if  ($for_dn) 
        $metaChars = array(',','=', '+', '<','>',';', '\\', '"', '#'); 
    else 
        $metaChars = array('*', '(', ')', '\\', chr(0)); 

    $quotedMetaChars = array(); 
    foreach ($metaChars as $key => $value) $quotedMetaChars[$key] = '\\'.str_pad(dechex(ord($value)), 2, '0');
    $str=str_replace($metaChars,$quotedMetaChars,$str); //replace them 
    return ($str); 
} 
?>