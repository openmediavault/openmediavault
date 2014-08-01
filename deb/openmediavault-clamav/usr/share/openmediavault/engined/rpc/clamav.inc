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
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceClamAV extends OMVRpcServiceAbstract {
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
	}

	/**
	 * Get configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//services/clamav";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return array(
			"enable" => boolvalEx($object['enable']),
			"checks" => intval($object['freshclam']['checks']),
			"databasemirror" => $object['freshclam']['databasemirror'],
			"quarantine.sharedfolderref" => $object[
			  'quarantine.sharedfolderref'],
			"logclean" => boolvalEx($object['clamd']['logclean']),
			"scanpe" => boolvalEx($object['clamd']['scanpe']),
			"scanole2" => boolvalEx($object['clamd']['scanole2']),
			"scanhtml" => boolvalEx($object['clamd']['scanhtml']),
			"scanpdf" => boolvalEx($object['clamd']['scanpdf']),
			"scanelf" => boolvalEx($object['clamd']['scanelf']),
			"scanarchive" => boolvalEx($object['clamd']['scanarchive']),
			"detectbrokenexecutables" => boolvalEx(
			  $object['clamd']['detectbrokenexecutables']),
			"algorithmicdetection" => boolvalEx(
			  $object['clamd']['algorithmicdetection']),
			"followdirectorysymlinks" => boolvalEx(
			  $object['clamd']['followdirectorysymlinks']),
			"followfilesymlinks" => boolvalEx(
			  $object['clamd']['followfilesymlinks']),
			"detectpua" => boolvalEx($object['clamd']['detectpua']),
			"extraoptions" => $object['clamd']['extraoptions']
		);
	}

	/**
	 * Set configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_CONFIG_UNMET_DEPENDENCY
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setSettings($params, $context) {
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
				  "databasemirror":{"type":"string"},
				  "checks":{"type":"integer","minimum":0,"maximum":50},
				  "quarantine.sharedfolderref":{'.$GLOBALS[
				    'OMV_JSONSCHEMA_UUID_NONE'].'},
				  "logclean":{"type":"boolean"},
				  "scanpe":{"type":"boolean"},
				  "scanole2":{"type":"boolean"},
				  "scanhtml":{"type":"boolean"},
				  "scanpdf":{"type":"boolean"},
				  "scanelf":{"type":"boolean"},
				  "scanarchive":{"type":"boolean"},
				  "detectbrokenexecutables":{"type":"boolean"},
				  "algorithmicdetection":{"type":"boolean"},
				  "followdirectorysymlinks":{"type":"boolean"},
				  "followfilesymlinks":{"type":"boolean"},
				  "detectpua":{"type":"boolean"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Get the existing configuration object.
		$xpath = "//services/clamav";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the existing configuration object.
		$object = array_merge($object, array(
			"enable" => array_boolval($params, "enable"),
			"quarantine.sharedfolderref" => $params[
			  'quarantine.sharedfolderref'],
			"clamd" => array(
				"logclean" => array_boolval($params, "logclean"),
				"scanpe" => array_boolval($params, "scanpe"),
				"scanole2" => array_boolval($params, "scanole2"),
				"scanhtml" =>array_boolval($params, "scanhtml"),
				"scanpdf" => array_boolval($params, "scanpdf"),
				"scanelf" => array_boolval($params, "scanelf"),
				"scanarchive" => array_boolval($params, "scanarchive"),
				"detectbrokenexecutables" => array_boolval($params,
				  "detectbrokenexecutables"),
				"algorithmicdetection" => array_boolval($params,
				  "algorithmicdetection"),
				"followdirectorysymlinks" => array_boolval($params,
				  "followdirectorysymlinks"),
				"followfilesymlinks" => array_boolval($params,
				  "followfilesymlinks"),
				"detectpua" => array_boolval($params, "detectpua"),
				"extraoptions" => $params['extraoptions']
			),
			"freshclam" => array(
				"enable" => ($params['checks'] > 0) ? TRUE : FALSE,
				"checks" => $params['checks'],
				"databasemirror" => $params['databasemirror']
			)
		));
		// Do some validations:
		// - Check if quarantine shared folder is referenced and required.
		if (empty($object['quarantine.sharedfolderref'])) {
			if ($xmlConfig->exists("//services/clamav/jobs/job[".
			  "virusaction='quarantine']")) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_UNMET_DEPENDENCY,
				  "The quarantine folder is required");
			}
		}
		// Update the configuration object.
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.clamav", $object);
		// Return the configuration object.
		return $object;
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
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getJobList($params, $context) {
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
		// Get the configuration objects.
		$xpath = "//services/clamav/jobs/job";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional information.
		foreach($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[".
			  "uuid='{$objectv['sharedfolderref']}']");
			if(!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
			// Modify result data.
			$objectv['enable'] = boolvalEx($objectv['enable']);
			$objectv['everynminute'] = boolvalEx($objectv['everynminute']);
			$objectv['everynhour'] = boolvalEx($objectv['everynhour']);
			$objectv['everyndayofmonth'] = boolvalEx($objectv['everyndayofmonth']);
			$objectv['sendemail'] = boolvalEx($objectv['sendemail']);
			$objectv['onaccess'] = boolvalEx($objectv['onaccess']);
			$objectv['multiscan'] = boolvalEx($objectv['multiscan']);
			$objectv['verbose'] = boolvalEx($objectv['verbose']);
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a virus scan job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getJob($params, $context) {
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
		$xpath = sprintf("//services/clamav/jobs/job[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['enable'] = boolvalEx($object['enable']);
		$object['everynminute'] = boolvalEx($object['everynminute']);
		$object['everynhour'] = boolvalEx($object['everynhour']);
		$object['everyndayofmonth'] = boolvalEx($object['everyndayofmonth']);
		$object['sendemail'] = boolvalEx($object['sendemail']);
		$object['onaccess'] = boolvalEx($object['onaccess']);
		$object['multiscan'] = boolvalEx($object['multiscan']);
		$object['verbose'] = boolvalEx($object['verbose']);
		return $object;
	}

	/**
	 * Set (add/update) a virus scan job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_UNMET_DEPENDENCY
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setJob($params, $context) {
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
				  "enable":{"type":"boolean"},
				  "sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "minute":{"type":"string","format":"regex",'.
					'"pattern":"\/^[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|'.
					'5[0-9]|[*]$\/"},
				  "everynminute":{"type":"boolean"},
				  "hour":{"type":"string","format":"regex",'.
					'"pattern":"\/^[0-9]|1[0-9]|2[0-3]|[*]$\/"},
				  "everynhour":{"type":"boolean"},
				  "month":{"type":"string","format":"regex",'.
					'"pattern":"\/^[1-9]|1[0-2]|[*]$\/"},
				  "dayofmonth":{"type":"string","format":"regex",'.
					'"pattern":"\/^[1-9]|1[0-9]|2[0-9]|3[0-1]|[*]$\/"},
				  "everyndayofmonth":{"type":"boolean"},
				  "dayofweek":{"type":"string","format":"regex",'.
					'"pattern":"\/^[1-7]|[*]$\/"},
				  "sendemail":{"type":"boolean"},
				  "onaccess":{"type":"boolean"},
				  "virusaction":{"type":"string","enum":["nothing",'.
				    '"quarantine","delete"]},
				  "multiscan":{"type":"boolean"},
				  "verbose":{"type":"boolean"},
				  "comment":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"enable" => array_boolval($params, 'enable'),
			"sharedfolderref" => $params['sharedfolderref'],
			"minute" => $params['minute'],
			"everynminute" => array_boolval($params, 'everynminute'),
			"hour" => $params['hour'],
			"everynhour" => array_boolval($params, 'everynhour'),
			"dayofmonth" => $params['dayofmonth'],
			"everyndayofmonth" => array_boolval($params, 'everyndayofmonth'),
			"month" => $params['month'],
			"dayofweek" => $params['dayofweek'],
			"sendemail" => array_boolval($params, 'sendemail'),
			"onaccess" => array_boolval($params, 'onaccess'),
			"virusaction" => $params['virusaction'],
			"multiscan" => array_boolval($params, 'multiscan'),
			"verbose" => array_boolval($params, 'verbose'),
			"comment" => $params['comment']
		);
		// Do some validations:
		// - Check if quarantine shared folder exists when it is referenced.
		if ("quarantine" === $object['virusaction']) {
			$settings = $this->callMethod("getSettings", NULL, $context);
			if (empty($settings['quarantine.sharedfolderref'])) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_UNMET_DEPENDENCY,
				  "No quarantine folder set");
			}
		}
		// Set configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//services/clamav/jobs",
			  array("job" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//services/clamav/jobs/" .
			  "job[uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.clamav.jobs.job", $object);
		// Return the configuration object.
		return $object;
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
		$xpath = sprintf("//services/clamav/jobs/job[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.clamav.jobs.job");
	}

	/**
	 * Execute a virus scan job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the job to execute.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_CONFIG_DIRTY
	 */
	public function executeJob($params, $context) {
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
		// that the rsync cron script has not been created or updated
		// until now.
		if ($this->isModuleDirty("clamav")) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_DIRTY);
		}
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if ($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Execute the clamdscan cron script.
			$cmd = "/bin/sh";
			$cmdArgs = array();
			$cmdArgs[] = sprintf("%s/clamdscan-%s",
			  $GLOBALS['OMV_CRONSCRIPTS_DIR'], $params['uuid']);
			if (FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd." ".implode(" ", $cmdArgs), "");
			}
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceClamAV());
?>
