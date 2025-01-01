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

class CertificateMgmt extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getSshList");
		$this->registerMethod("createSsh");
		$this->registerMethod("getSsh");
		$this->registerMethod("setSsh");
		$this->registerMethod("deleteSsh");
		$this->registerMethod("copySshId");
	}

	/**
	 * Get list of SSL certificate configuration objects.
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
	public function getList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get list of SSL certificate configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.certificate.ssl");
		// Process the list of objects.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			$certificate = $objectv->get("certificate");
			// Add additional properties.
			$objectv->add("name", "string", "");
			$objectv->add("validfrom", "integer", 0);
			$objectv->add("validto", "integer", 0);
			$objectv->add("_used", "boolean", FALSE);
			// Remove the private key. It should not leave the system.
			$objectv->remove("privatekey");
			// Parse the certificate.
			if ($certinfo = openssl_x509_parse($certificate)) {
				$objectv->setFlatAssoc([
					"name" => $certinfo['name'],
					"validfrom" => $certinfo['validFrom_time_t'],
					"validto" => $certinfo['validTo_time_t']
				]);
			}
			// Get the fingerprints.
			foreach ([ "sha1", "sha256" ] as $digestAlgo) {
				if (FALSE !== ($fingerprint = openssl_x509_fingerprint(
						$certificate, $digestAlgo))) {
					$name = sprintf("fingerprint%s", $digestAlgo);
					$objectv->add($name, "string", "");
					$objectv->set($name, substr(chunk_split(
						$fingerprint, 2, ':'), 0, -1));
				}
			}
			// Set '_used' flag if the certificate is referenced.
			$objectv->set("_used", $db->isReferenced($objectv));
			// Get the values.
			$result[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($result, $params['start'], $params['limit'],
			$params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a SSL certificate configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.certificate.ssl", $params['uuid']);
		// Remove the private key. It should not leave the system.
		$object->remove("privatekey");
		// Return the values.
		return $object->getAssoc();
	}

	/**
	 * Set (add/update) a SSL certificate configuration object
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.certificatemgmt.set");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.certificate.ssl");
		$object->setAssoc($params);
		// Validate the certificate.
		if (!($certificate = openssl_x509_read($object->get("certificate")))) {
			throw new \OMV\Exception("Invalid certificate: %s",
			  openssl_error_string());
		}
		// Validate the private key.
		if (!$object->isEmpty("privatekey")) {
			if (!($privatekey = openssl_pkey_get_private(
			  $object->get("privatekey")))) {
				openssl_x509_free($certificate);
				throw new \OMV\Exception("Invalid private key: %s",
				  openssl_error_string());
			}
			// Check if the private key corresponds to the certificate.
			if (!openssl_x509_check_private_key($certificate, $privatekey)) {
				openssl_pkey_free($privatekey);
				openssl_x509_free($certificate);
				throw new \OMV\Exception(
				  "Private key does not correspond to the certificate: %s",
				  openssl_error_string());
			}
			openssl_pkey_free($privatekey);
		}
		openssl_x509_free($certificate);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		// The private key is required here.
		if ($object->isEmpty("privatekey")) {
			if (TRUE === $object->isNew()) {
				throw new \OMV\Exception("Private key does not exist.");
			} else {
				$oldObject = $db->get("conf.system.certificate.ssl",
					$object->getIdentifier());
				$object->set("privatekey", $oldObject->get("privatekey"));
			}
		}
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a SSL certificate config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function delete($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Make sure the certificate is not referenced.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.certificate.ssl", $params['uuid']);
		$db->assertIsNotReferenced($object);
		// Delete the configuration object.
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get detail about a SSL certificate.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return A string containing the certificate details.
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	public function getDetail($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Check if the module is marked as dirty. This is an indication
		// that the certificate has not been created or updated until now.
		if ($this->isModuleDirty("certificates"))
			throw new \OMV\Config\ConfigDirtyException();
		$cmdArgs = [];
		$cmdArgs[] = "x509";
		$cmdArgs[] = "-noout";
		$cmdArgs[] = sprintf("-in %s", escapeshellarg(sprintf(
			"%s/certs/%s%s.crt",
			\OMV\Environment::get("OMV_SSL_CERTIFICATE_DIR"),
			\OMV\Environment::get("OMV_SSL_CERTIFICATE_PREFIX"),
			$params['uuid'])));
		// Get certificate details.
		$cmd = new \OMV\System\Process("openssl", array_merge($cmdArgs,
			[ "-text", "-nameopt utf8" ]));
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Append the certificate fingerprints.
		$indent = str_repeat(" ", 4);
		$fingerprintOutput = [ "Fingerprints:" ];
		foreach ([ "-sha1", "-sha256" ] as $digest) {
			$cmd = new \OMV\System\Process("openssl", array_merge(
				$cmdArgs, [ "-fingerprint", $digest ]));
			$cmd->setRedirect2to1();
			$fingerprintOutput[] = $indent . str_replace(
				"=", ":\n" . $indent . $indent,
				$cmd->execute());
		}
		return implode("\n", array_merge($output, $fingerprintOutput));
	}

	/**
	 * Create a SSL certificate.
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
	 */
	public function create($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.certificatemgmt.create");
		// Generate the certificate subject string.
		$rdnFields = [ "c", "st", "l", "o", "ou", "cn", "email" ];
		$subject = "";
		foreach ($rdnFields as $rdnFieldk => $rdnFieldv) {
			if (empty($params[$rdnFieldv]))
				continue;
			// Escape RDN attribute value, see
			// http://www-03.ibm.com/systems/i/software/ldap/underdn.html
			// http://msdn.microsoft.com/en-us/library/windows/desktop/aa366101(v=vs.85).aspx
			$replace = [];
			foreach ([ ',', '+', '<', '>', ';', '\r', '=', '#', '/' ] as $v) {
				$replace[$v] = sprintf("\\%s", mb_strtoupper(str_pad(dechex(
				  ord($v)), 2, "0")));
			}
			$params[$rdnFieldv] = strtr($params[$rdnFieldv], $replace);
			$params[$rdnFieldv] = addcslashes($params[$rdnFieldv], '"\\');
			// Append to subject.
			switch ($rdnFieldv) {
			case "email":
				// Append the email address.
				$subject .= sprintf("/emailAddress=%s", $params['email']);
				break;
			default:
				$subject .= sprintf("/%s=%s", mb_strtoupper($rdnFieldv),
				  $params[$rdnFieldv]);
			}
		}
		// Generate the certificate Subject Alternative Name (SAN) array.
		$subjectAltName = [];
		if (!empty($params['cn'])) {
			$subjectAltName[] = sprintf("%s:%s",
				is_ipaddr($params['cn']) ? "IP" : "DNS",
				$params['cn']);
		}
		if (!empty($params['email'])) {
			$subjectAltName[] = "email:copy";
		}
		// Create the requested certificate.
		// http://www.zytrax.com/tech/survival/ssl.html
		$keyFile = tempnam(sys_get_temp_dir(), "key");
		$crtFile = tempnam(sys_get_temp_dir(), "crt");
		$cmdArgs = [];
		$cmdArgs[] = "req";
		$cmdArgs[] = "-new";
		$cmdArgs[] = "-x509";
		$cmdArgs[] = "-nodes";
		$cmdArgs[] = sprintf("-days %d", $params['days']);
		$cmdArgs[] = "-sha256";
		$cmdArgs[] = "-utf8";
		$cmdArgs[] = "-batch";
		$cmdArgs[] = sprintf("-newkey rsa:%d", $params['size']);
		$cmdArgs[] = sprintf("-keyout %s", escapeshellarg($keyFile));
		$cmdArgs[] = sprintf("-out %s", escapeshellarg($crtFile));
		if (!empty($subject)) {
			$cmdArgs[] = sprintf("-subj %s", escapeshellarg($subject));
		}
		if (!empty($subjectAltName)) {
			$cmdArgs[] = sprintf("-addext 'subjectAltName=%s'",
				join(",", $subjectAltName));
		}
		$cmd = new \OMV\System\Process("openssl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		if (0 !== $exitStatus) {
			unlink($keyFile);
			unlink($crtFile);
			throw new \OMV\ExecException($cmd, $output, $exitStatus);
		}
		// Read certificate and key content, then unlink the temporary files.
		$keyData = file_get_contents($keyFile);
		$crtData = file_get_contents($crtFile);
		unlink($keyFile);
		unlink($crtFile);
		// Finally import generated certificate.
		return $this->callMethod("set", [
			"uuid" => \OMV\Environment::get("OMV_CONFIGOBJECT_NEW_UUID"),
			"certificate" => $crtData,
			"privatekey" => $keyData,
			"comment" => !empty($params['comment']) ?
				$params['comment'] : $subject
		], $context);
	}

	/**
	 * Get list of SSH certificate configuration objects.
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
	public function getSshList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get list of SSH certificate configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.certificate.ssh");
		// Process the list of objects.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			// Add additional properties.
			$objectv->add("_used", "boolean");
			// Remove the private key. It should not leave the system.
			$objectv->remove("privatekey");
			// Set '_used' flag if the certificate is referenced.
			$objectv->set("_used", $db->isReferenced($objectv));
			// Get the values.
			$result[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($result, $params['start'], $params['limit'],
		  $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Create a SSH certificate.
	 * @param params An array containing the following fields:
	 *   \em comment The comment
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function createSsh($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.certificatemgmt.createssh");
		// Create the requested certificate.
		$file = tempnam(sys_get_temp_dir(), "id_".$params['type']);
		$_ = defer(function() use ($file) {
			@unlink($file);
			@unlink($file.".pub");
		});
		$cmdArgs = [];
		$cmdArgs[] = sprintf("-t %s", escapeshellarg($params['type']));
		$cmdArgs[] = "-m PEM";
		$cmdArgs[] = "-P ''";
		$cmdArgs[] = sprintf("-C %s", escapeshellarg(trim(array_value(
			$params, "comment", ""))));
		$cmdArgs[] = sprintf("-f %s", escapeshellarg($file));
		$cmd = sprintf("export LC_ALL=C.UTF-8; (echo y) | ssh-keygen %s 2>&1",
			implode(" ", $cmdArgs));
		if (0 !== ($exitStatus = $this->exec($cmd, $output))) {
			throw new \OMV\ExecException($cmd, $output, $exitStatus);
		}
		// Read certificate and key content, then unlink the temporary files.
		$privateKey = file_get_contents($file);
		$publicKey = file_get_contents($file.".pub");
		// Finally import generated certificate.
		return $this->callMethod("setSsh", [
			"uuid" => \OMV\Environment::get("OMV_CONFIGOBJECT_NEW_UUID"),
			"publickey" => trim($publicKey),
			"privatekey" => trim($privateKey),
			"comment" => array_value($params, "comment", "")
		], $context);
	}

	/**
	 * Get a SSH certificate configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object. Note, the
	 *   private key is not included in the response because of
	 *   security issues.
	 */
	function getSsh($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.certificate.ssh", $params['uuid']);
		// Remove the private key. It should not leave the system.
		$object->remove("privatekey");
		// Return the values.
		return $object->getAssoc();
	}

	/**
	 * Set (add/update) a SSH certificate configuration object
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setSsh($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.certificatemgmt.setssh");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.certificate.ssh");
		$object->setAssoc($params);
		// Validate the private key.
		if (!$object->isEmpty("privatekey")) {
			// ToDo...
		}
		// Set the configuration data.
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			// The private key is required here.
			if ($object->isEmpty("privatekey")) {
				throw new \OMV\Exception("Private key does not exist.");
			}
		} else {
			$oldObject = $db->get("conf.system.certificate.ssh",
				$object->getIdentifier());
			$object->set("privatekey", $oldObject->get("privatekey"));
		}
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a SSH certificate config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteSsh($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Make sure the certificate is not referenced.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.certificate.ssh", $params['uuid']);
		$db->assertIsNotReferenced($object);
		// Delete the configuration object.
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Copy the public key of a SSH certificate to a remote machine.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em hostname The hostname of the remote machine.
	 *   \em port The SSH port of the remote machine.
	 *   \em username The name of the user.
	 *   \em password The user password.
	 * @param context The context of the caller.
	 */
	public function copySshId($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.certificatemgmt.copySshId");
		// Check if the module is marked as dirty. Abort immediately if
		// that is the case because the specified SSH certificate might
		// not exist then.
		if ($this->isModuleDirty("certificates")) {
			throw new \OMV\Config\ConfigDirtyException();
		}
		// Simply ensure the specified SSH certificate exists.
		$db = \OMV\Config\Database::getInstance();
		$db->get("conf.system.certificate.ssh", $params['uuid']);
		// Create the temporary file that contains the password.
		$pwdFile = new \OMV\System\TmpFile();
		$pwdFile->write($params['password']);
		// Get the home directory from user 'root'.
		$user = new \OMV\System\User("root");
		// Build the command to copy the public SSH key to the remote system.
		$cmdArgs = [];
		$cmdArgs[] = "-v";
		$cmdArgs[] = "-f";
		$cmdArgs[] = escapeshellarg($pwdFile->getFilename());
		$cmdArgs[] = "--";
		$cmdArgs[] = "ssh-copy-id";
		$cmdArgs[] = "-o";
		$cmdArgs[] = "StrictHostKeyChecking=no";
		$cmdArgs[] = "-i";
		$cmdArgs[] = escapeshellarg(sprintf("%s/%s%s.pub",
			\OMV\Environment::get("OMV_SSH_KEYS_DIR", "/etc/ssh"),
			\OMV\Environment::get("OMV_SSH_KEY_PREFIX", "openmediavault-"),
			$params['uuid']));
		$cmdArgs[] = "-p";
		$cmdArgs[] = escapeshellarg($params['port']);
		$cmdArgs[] = escapeshellarg(sprintf("%s@%s", $params['username'],
			$params['hostname']));
		$cmd = new \OMV\System\Process("sshpass", $cmdArgs);
		$cmd->setEnv("HOME", $user->getHomeDirectory());
		$cmd->setRedirect2to1();
		$cmd->execute();
	}
}
