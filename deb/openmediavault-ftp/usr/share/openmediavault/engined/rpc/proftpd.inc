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

class OMVRpcServiceProftpd extends \OMV\Rpc\ServiceAbstract {
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.ftp.share");
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
	 */
	public function setShare($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.ftp.setshare");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.share");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		// Check uniqueness:
		// - A shared folder can only be configured once for FTP.
		$db->assertIsUnique($object, "sharedfolderref");
		$db->set($object);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ftp.share", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
	 */
	public function getModBanRule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
	 */
	public function setModBanRule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.ftp.setmodbanrule");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.modban.rule");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a mod_ban rule config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 */
	public function deleteModBanRule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ftp.modban.rule", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get the TLS settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function getModTLSSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.ftp.modtls");
	}

	/**
	 * Set the TLS settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setModTLSSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.ftp.setmodtlssettings");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.modtls");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get statistics, e.g. who's online.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A string containing the requested statistics.
	 */
	public function getStats($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Is service enabled?
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ftp");
		if (FALSE === $object->get("enable")) {
			$stats = gettext("Service disabled");
		} else {
			// Raise an exception if the module is marked as dirty,
			// otherwise an exec exception is raised when the 'ftpwho'
			// command is executed because the FTP service has not been
			// deployed until now.
			if ($this->isModuleDirty("proftpd"))
				throw new \OMV\Config\ConfigDirtyException();
			// Get users currently logged in.
			$stats = sprintf("%s\n", gettext("Currently logged in users:"));
			$cmdArgs = [];
			$cmdArgs[] = "-v";
			$cmd = new \OMV\System\Process("ftpwho", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			array_shift($output);
			$stats .= implode("\n", $output);
			// Get ban list if module is enabled.
			if (TRUE === $db->exists("conf.service.ftp.modban.rule")) {
				$stats .= sprintf("\n\n%s\n", gettext("Ban statistics:"));
				$cmdArgs = [];
				$cmdArgs[] = sprintf("-s %s", \OMV\Environment::get(
					"OMV_PROFTPD_MODCTRLS_CONTROLSSOCKET",
					"/run/proftpd/proftpd.sock"));
				$cmdArgs[] = "ban info";
				$cmdArgs[] = "-v";
				$cmdArgs[] = "-e";
				$output = [];
				$cmd = new \OMV\System\Process("ftpdctl", $cmdArgs);
				$cmd->setRedirect2to1();
				$cmd->execute($output);
				foreach ($output as &$outputv) {
					$outputv = trim(preg_replace("/^ftpdctl:/", "",
						$outputv));
				}
				$stats .= implode("\n", $output);
			}
		}
		return $stats;
	}
}
