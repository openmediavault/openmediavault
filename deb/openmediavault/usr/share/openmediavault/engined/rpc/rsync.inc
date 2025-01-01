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

require_once("openmediavault/functions.inc");

class Rsync extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Rsync";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("execute");
	}

	/**
	 * Get list of rsync job configuration objects.
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
		// Get configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.rsync.job");
		// Add additional information.
		$objectsAssoc = [];
		foreach ($objects as $objectk => &$objectv) {
			// Add additional properties.
			$objectv->add("srcname", "string", gettext("n/a"));
			$objectv->add("destname", "string", gettext("n/a"));
			// Set the prooperties depending on the 'type' property.
			switch ($objectv->get("type")) {
			case "local":
				// Get the shared folder configuration object and update the
				// 'srcname' property.
				$sfObject = $db->get("conf.system.sharedfolder",
				  $objectv->get("src.sharedfolderref"));
				$objectv->set("srcname", $sfObject->get("name"));
				// Get the shared folder configuration object and update the
				// 'destname' property.
				$sfObject = $db->get("conf.system.sharedfolder",
				  $objectv->get("dest.sharedfolderref"));
				$objectv->set("destname", $sfObject->get("name"));
				break;
			case "remote":
				switch ($objectv->get("mode")) {
				case "push":
					// Get the shared folder config object.
					$sfObject = $db->get("conf.system.sharedfolder",
					  $objectv->get("src.sharedfolderref"));
					// Set the additional 'srcname' and 'destname' properties.
					$objectv->set("srcname", $sfObject->get("name"));
					$objectv->copy("dest.uri", "destname");
					break;
				case "pull":
					// Get the shared folder config object.
					$sfObject = $db->get("conf.system.sharedfolder",
					  $objectv->get("dest.sharedfolderref"));
					// Set the additional 'srcname' and 'destname' properties.
					$objectv->set("destname", $sfObject->get("name"));
					$objectv->copy("src.uri", "srcname");
					break;
				}
				break;
			}
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a rsync job configuration object.
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
		$object = $db->get("conf.service.rsync.job", $params['uuid']);
		// Add additional information.
		switch ($object->get("type")) {
		case "local":
			$object->copy("src.sharedfolderref", "srcsharedfolderref");
			$object->copy("dest.sharedfolderref", "destsharedfolderref");
			break;
		case "remote":
			switch ($object->get("mode")) {
			case "push":
				$object->copy("src.sharedfolderref", "srcsharedfolderref");
				$object->copy("dest.uri", "desturi");
				break;
			case "pull":
				$object->copy("src.uri", "srcuri");
				$object->copy("dest.sharedfolderref", "destsharedfolderref");
				break;
			}
			break;
		}
		$result = $object->getAssoc();
		// Convert comma separated list into an array.
		$result['minute'] = explode(",", $result['minute']);
		$result['hour'] = explode(",", $result['hour']);
		$result['dayofmonth'] = explode(",", $result['dayofmonth']);
		$result['month'] = explode(",", $result['month']);
		$result['dayofweek'] = explode(",", $result['dayofweek']);
		return $result;
	}

	/**
	 * Set a rsync job configuration object.
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
		$this->validateMethodParams($params, "rpc.rsync.set");
		// Convert array into a comma separated list.
		$params['minute'] = implode(",", $params['minute']);
		$params['hour'] = implode(",", $params['hour']);
		$params['dayofmonth'] = implode(",", $params['dayofmonth']);
		$params['month'] = implode(",", $params['month']);
		$params['dayofweek'] = implode(",", $params['dayofweek']);
		// Prepare the configuration data.
		$object = new \OMV\Config\ConfigObject("conf.service.rsync.job");
		$object->setAssoc($params, TRUE, TRUE);
		switch ($params['type']) {
		case "local":
			$object->set("src.sharedfolderref", $params[
				'srcsharedfolderref']);
			$object->set("src.uri", "");
			$object->set("dest.sharedfolderref", $params[
				'destsharedfolderref']);
			$object->set("dest.uri", "");
			break;
		case "remote":
			switch ($params['mode']) {
			case "push":
				$object->set("src.sharedfolderref", $params[
					'srcsharedfolderref']);
				$object->set("src.uri", "");
				$object->set("dest.sharedfolderref", "");
				$object->set("dest.uri", $params['desturi']);
				break;
			case "pull":
				$object->set("src.sharedfolderref", "");
				$object->set("src.uri", $params['srcuri']);
				$object->set("dest.sharedfolderref", $params[
					'destsharedfolderref']);
				$object->set("dest.uri", "");
				break;
			}
			break;
		}
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a rsync job configuration object.
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
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.rsync.job", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Execute a rsync job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the cron job to execute.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	public function execute($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Check if the module is marked as dirty. This is an indication
		// that the rsync cron scripts have not been created or updated
		// until now.
		if ($this->isModuleDirty("rsync"))
			throw new \OMV\Config\ConfigDirtyException();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
				use ($params) {
			// Execute the rsync cron script.
			$cmdArgs = [];
			$cmdArgs[] = "--shell";
			$cmdArgs[] = "--non-interactive";
			$cmdArgs[] = "--";
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR,
				\OMV\Environment::get("OMV_CRONSCRIPTS_DIR"),
				sprintf("rsync-%s", $params['uuid']));
			$cmd = new \OMV\System\Process("sudo", $cmdArgs);
			$cmd->setEnv("SHELL", "/bin/sh");
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}
}
