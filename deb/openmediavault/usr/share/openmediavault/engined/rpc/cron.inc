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

class Cron extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Cron";
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
	 * Get the cron jobs of the requested type.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 *   \em type Array of cron jobs types which are reboot, shutdown,
	 *   standby or userdefined.
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
		$this->validateMethodParams($params, "rpc.cron.getlist");
		// Get configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.cron.job");
		// Get cron jobs that match the given type.
		$jobs = [];
		foreach ($objects as $objectk => $objectv) {
			if (in_array($objectv->get("type"), $params['type'])) {
				$jobs[] = $objectv->getAssoc();
			}
		}
		// Filter the result.
		return $this->applyFilter($jobs, $params['start'], $params['limit'],
		  $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a cron job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$result = $db->getAssoc("conf.system.cron.job", $params['uuid']);
		// Convert comma separated list into an array.
		$result['minute'] = explode(",", $result['minute']);
		$result['hour'] = explode(",", $result['hour']);
		$result['dayofmonth'] = explode(",", $result['dayofmonth']);
		$result['month'] = explode(",", $result['month']);
		$result['dayofweek'] = explode(",", $result['dayofweek']);
		return $result;
	}

	/**
	 * Set (add/update) a cron job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.cron.set");
		// Convert array into a comma separated list.
		$params['minute'] = implode(",", $params['minute']);
		$params['hour'] = implode(",", $params['hour']);
		$params['dayofmonth'] = implode(",", $params['dayofmonth']);
		$params['month'] = implode(",", $params['month']);
		$params['dayofweek'] = implode(",", $params['dayofweek']);
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.cron.job");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a cron job configuration object.
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
		$object = $db->get("conf.system.cron.job", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Execute a cron job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	public function execute($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Check if the module is marked as dirty. This is an indication
		// that the user-defined cron scripts have not been created or
		// updated until now.
		if ($this->isModuleDirty("cron"))
			throw new \OMV\Config\ConfigDirtyException();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
				use ($params) {
			// Get the configuration object.
			$db = \OMV\Config\Database::getInstance();
			$object = $db->get("conf.system.cron.job", $params['uuid']);
			// Execute the user-defined cron script.
			// Note, we need to set the shell, otherwise the users shell
			// is used instead of the shebang of the command script.
			$cmdArgs = [];
			$cmdArgs[] = "--shell";
			$cmdArgs[] = "--non-interactive";
			$cmdArgs[] = sprintf("--user=%s", escapeshellarg(
				$object->get("username")));
			$cmdArgs[] = "--";
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR,
				\OMV\Environment::get("OMV_CRONSCRIPTS_DIR"),
				sprintf("userdefined-%s", $params['uuid']));
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
