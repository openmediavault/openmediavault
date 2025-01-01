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
class OMVRpcServiceClamAV extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "ClamAV";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getJobList");
		$this->registerMethod("getJob");
		$this->registerMethod("setJob");
		$this->registerMethod("deleteJob");
		$this->registerMethod("executeJob");
		$this->registerMethod("getOnAccessPathList");
		$this->registerMethod("getOnAccessPath");
		$this->registerMethod("setOnAccessPath");
		$this->registerMethod("deleteOnAccessPath");
		$this->registerMethod("getStats");
	}

	/**
	 * Get configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.clamav");
		// Prepare the result.
		$result = $object->get("clamd");
		$result['enable'] = $object->get("enable");
		$result['checks'] = $object->get("freshclam.checks");
		$result['quarantine_sharedfolderref'] = $object->get(
		  "quarantine.sharedfolderref");
		return $result;
	}

	/**
	 * Set configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.clamav.setsettings");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.clamav");
		$object->setFlatAssoc([
			"enable" => $params['enable'],
			"quarantine.sharedfolderref" => $params[
			  'quarantine_sharedfolderref'],
			"clamd.logclean" => $params['logclean'],
			"clamd.scanpe" => $params['scanpe'],
			"clamd.scanole2" => $params['scanole2'],
			"clamd.scanhtml" => $params['scanhtml'],
			"clamd.scanpdf" => $params['scanpdf'],
			"clamd.scanelf" => $params['scanelf'],
			"clamd.scanarchive" => $params['scanarchive'],
			"clamd.detectbrokenexecutables" => $params[
			  'detectbrokenexecutables'],
			"clamd.alertbrokenmedia" => $params['alertbrokenmedia'],
			"clamd.algorithmicdetection" => $params['algorithmicdetection'],
			"clamd.followdirectorysymlinks" => $params[
			  'followdirectorysymlinks'],
			"clamd.detectpua" => $params['detectpua'],
			"clamd.extraoptions" => $params['extraoptions'],
			"freshclam.enable" => $params['enable'] && $params['checks'] > 0,
			"freshclam.checks" => $params['checks']
		]);
		// Do some validations:
		// - Check if quarantine shared folder is referenced and required.
		if (TRUE === $object->isEmpty("quarantine.sharedfolderref")) {
			if (TRUE === $db->exists("conf.service.clamav.job", [
				"operator" => "stringEquals",
				"arg0" => "virusaction",
				"arg1" => "quarantine"
			])) {
				throw new \OMV\Exception("The quarantine folder is required.");
			}
		}
		// Put the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get list of virus scan job configuration objects.
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
	public function getJobList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.clamav.job");
		// Add additional information.
		$objectsAssoc = [];
		foreach ($objects as $objectk => $objectv) {
			// Add the new property 'sharedfoldername'.
			$objectv->add("sharedfoldername", "string", gettext("n/a"));
			// Get the shared folder configuration object.
			$sfObject = $db->get("conf.system.sharedfolder",
			  $objectv->get("sharedfolderref"));
			// Update the 'sharedfoldername' property.
			$objectv->set("sharedfoldername", $sfObject->get("name"));
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a virus scan job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getJob($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$result = $db->getAssoc("conf.service.clamav.job", $params['uuid']);
		// Convert comma separated list into an array.
		$result['minute'] = explode(",", $result['minute']);
		$result['hour'] = explode(",", $result['hour']);
		$result['dayofmonth'] = explode(",", $result['dayofmonth']);
		$result['month'] = explode(",", $result['month']);
		$result['dayofweek'] = explode(",", $result['dayofweek']);
		return $result;
	}

	/**
	 * Set (add/update) a virus scan job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setJob($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.clamav.setjob");
		// Convert array into a comma separated list.
		$params['minute'] = implode(",", $params['minute']);
		$params['hour'] = implode(",", $params['hour']);
		$params['dayofmonth'] = implode(",", $params['dayofmonth']);
		$params['month'] = implode(",", $params['month']);
		$params['dayofweek'] = implode(",", $params['dayofweek']);
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.clamav.job");
		$object->setAssoc($params);
		// Do some validations:
		// - Check if quarantine shared folder exists when it is referenced.
		$db = \OMV\Config\Database::getInstance();
		if ("quarantine" === $object->get("virusaction")) {
			$object2 = $db->get("conf.service.clamav");
			if ($object2->isEmpty("quarantine.sharedfolderref")) {
				throw new \OMV\Exception("No quarantine folder is set.");
			}
		}
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a virus scan job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteJob($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.clamav.job", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Execute a virus scan job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the job to execute.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	public function executeJob($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Check if the module is marked as dirty. This is an indication
		// that the rsync cron script has not been created or updated
		// until now.
		if ($this->isModuleDirty("clamav"))
			throw new \OMV\Config\ConfigDirtyException();
		// Make sure the service is enabled.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.clamav");
		if (FALSE === $object->get("enable")) {
			throw new \OMV\Exception(
				"Failed to run scheduled job because the service is disabled.");
		}
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// Execute the clamdscan cron script.
			$cmdArgs = [];
			$cmdArgs[] = "--shell";
			$cmdArgs[] = "--non-interactive";
			$cmdArgs[] = "--";
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR,
				\OMV\Environment::get("OMV_CRONSCRIPTS_DIR"),
				sprintf("clamdscan-%s", $params['uuid']));
			$cmd = new \OMV\System\Process("sudo", $cmdArgs);
			$cmd->setEnv("SHELL", "/usr/bin/dash");
			$cmd->setRedirect2to1();
			$cmdLine = $cmd->getCommandLine();
			// Execute the command.
			$exitStatus = $this->exec($cmdLine, $output, $bgOutputFilename);
			// Check the exit status.
			if (0 !== $exitStatus) {
				throw new \OMV\ExecException($cmdLine, $output, $exitStatus);
			}
			return $output;
		});
	}

	/**
	 * Get the list of shared folders for on-access scanning.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of shared folder objects.
	 */
	public function getOnAccessPathList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.clamav.onaccesspath");
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
	 * Get a on-access scanning path configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getOnAccessPath($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.clamav.onaccesspath",
			$params['uuid']);
	}

	/**
	 * Set a on-access scanning path configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em enable TRUE if this on-access scanning directory enabled,
	 *     otherwise FALSE.
	 *   \em sharedfolderref The UUID of the shared folder configuration
	 *     object.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setOnAccessPath($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.clamav.setOnAccessPath");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject(
			"conf.service.clamav.onaccesspath");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a on-access scanning path configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteOnAccessPath($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.clamav.onaccesspath", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get statistics.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A string containing the requested statistics.
	 */
	public function getStats($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.clamav");
		if (FALSE === $object->get("enable")) {
			$stats = gettext("Service disabled");
		} else {
			$cmdArgs = [];
			$cmd = new \OMV\System\Process("clamconf", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			array_shift($output);
			array_shift($output);
			$stats = implode("\n", $output);
		}
		return $stats;
	}
}
