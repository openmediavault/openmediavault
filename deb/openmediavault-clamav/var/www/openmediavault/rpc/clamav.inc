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
require_once("openmediavault/error.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class ClamAVRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"databasemirror":{"type":"string"},
					"checks":{"type":"integer","minimum":0,"maximum":50},
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
			}'),
			"getJobList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getJob" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setJob" => array('{
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
					"remove":{"type":"boolean"},
					"multiscan":{"type":"boolean"},
					"verbose":{"type":"boolean"},
					"comment":{"type":"string"}
				}
			}'),
			"deleteJob" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"executeJob" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}')
		);
	}

	/**
	 * Get configuration object.
	 * @return An object containing the fields \em enable, \em checks,
	 *   \em databasemirror and \em extraoptions.
	 */
	function getSettings() {
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object.
		$xpath = "//services/clamav";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return array(
			"enable" => boolval($object['enable']),
			"checks" => intval($object['freshclam']['checks']),
			"databasemirror" => $object['freshclam']['databasemirror'],
			"scanpe" => boolval($object['clamd']['scanpe']),
			"scanole2" => boolval($object['clamd']['scanole2']),
			"scanhtml" => boolval($object['clamd']['scanhtml']),
			"scanpdf" => boolval($object['clamd']['scanpdf']),
			"scanelf" => boolval($object['clamd']['scanelf']),
			"scanarchive" => boolval($object['clamd']['scanarchive']),
			"detectbrokenexecutables" => boolval(
			  $object['clamd']['detectbrokenexecutables']),
			"algorithmicdetection" => boolval(
			  $object['clamd']['algorithmicdetection']),
			"followdirectorysymlinks" => boolval(
			  $object['clamd']['followdirectorysymlinks']),
			"followfilesymlinks" => boolval(
			  $object['clamd']['followfilesymlinks']),
			"detectpua" => boolval($object['clamd']['detectpua']),
			"extraoptions" => $object['clamd']['extraoptions']
		);
	}

	/**
	 * Set configuration object.
	 * @param data An array containing the fields \em enable.
	 */
	function setSettings($data) {
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
		// Get existing configuration object.
		$xpath = "//services/clamav";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update existing configuration object.
		$object = array_merge($object, array(
			"enable" => array_boolval($data, 'enable'),
			"clamd" => array(
				"scanpe" => $data['scanpe'],
				"scanole2" => $data['scanole2'],
				"scanhtml" => $data['scanhtml'],
				"scanpdf" => $data['scanpdf'],
				"scanelf" => $data['scanelf'],
				"scanarchive" => $data['scanarchive'],
				"detectbrokenexecutables" => $data['detectbrokenexecutables'],
				"algorithmicdetection" => $data['algorithmicdetection'],
				"followdirectorysymlinks" => $data['followdirectorysymlinks'],
				"followfilesymlinks" => $data['followfilesymlinks'],
				"detectpua" => $data['detectpua'],
				"extraoptions" => $data['extraoptions']
			),
			"freshclam" => array(
				"enable" => ($data['checks'] > 0) ? TRUE : FALSE,
				"checks" => $data['checks'],
				"databasemirror" => $data['databasemirror']
			)
		));
		// Set configuration object.
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration.
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.clamav", $object);
	}

	/**
	 * Get list of virus scan job configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getJobList($data) {
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
		$xpath = "//services/clamav/jobs/job";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations
		foreach ($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[".
			  "uuid='{$objectv['sharedfolderref']}']");
			if (!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
			// Modify result data
			$objectv['enable'] = boolval($objectv['enable']);
			$objectv['everynminute'] = boolval($objectv['everynminute']);
			$objectv['everynhour'] = boolval($objectv['everynhour']);
			$objectv['everyndayofmonth'] = boolval($objectv['everyndayofmonth']);
			$objectv['sendemail'] = boolval($objectv['sendemail']);
			$objectv['onaccess'] = boolval($objectv['onaccess']);
			$objectv['remove'] = boolval($objectv['remove']);
			$objectv['multiscan'] = boolval($objectv['multiscan']);
			$objectv['verbose'] = boolval($objectv['verbose']);
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a virus scan job configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	function getJob($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = sprintf("//services/clamav/jobs/job[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['everynminute'] = boolval($object['everynminute']);
		$object['everynhour'] = boolval($object['everynhour']);
		$object['everyndayofmonth'] = boolval($object['everyndayofmonth']);
		$object['sendemail'] = boolval($object['sendemail']);
		$object['onaccess'] = boolval($object['onaccess']);
		$object['remove'] = boolval($object['remove']);
		$object['multiscan'] = boolval($object['multiscan']);
		$object['verbose'] = boolval($object['verbose']);
		return $object;
	}

	/**
	 * Set (add/update) a virus scan job configuration object.
	 */
	function setJob($data) {
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
			"enable" => array_boolval($data, 'enable'),
			"sharedfolderref" => $data['sharedfolderref'],
			"minute" => $data['minute'],
			"everynminute" => array_boolval($data, 'everynminute'),
			"hour" => $data['hour'],
			"everynhour" => array_boolval($data, 'everynhour'),
			"dayofmonth" => $data['dayofmonth'],
			"everyndayofmonth" => array_boolval($data, 'everyndayofmonth'),
			"month" => $data['month'],
			"dayofweek" => $data['dayofweek'],
			"sendemail" => array_boolval($data, 'sendemail'),
			"onaccess" => array_boolval($data, 'onaccess'),
			"remove" => array_boolval($data, 'remove'),
			"multiscan" => array_boolval($data, 'multiscan'),
			"verbose" => array_boolval($data, 'verbose'),
			"comment" => $data['comment']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//services/clamav/jobs",
			  array("job" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//services/clamav/jobs/" .
			  "job[uuid='{$data['uuid']}']", $object);
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
		  "org.openmediavault.services.clamav.jobs.job", $object);
	}

	/**
	 * Delete a virus scan job configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function deleteJob($data) {
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
		$xpath = sprintf("//services/clamav/jobs/job[uuid='%s']",
		  $data['uuid']);
		$this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.clamav.jobs.job");
	}

	/**
	 * Execute a virus scan job.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the job to execute.
	 * @return The process id of the executed command. This is required by
	 *   the 'Exec' RPC for identification.
	 */
	public function executeJob($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the rsync job configuration object
		$object = OMVRpc::exec("ClamAV", "getJob", array(
		  "uuid" => $data['uuid']));
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Execute the cron command line
		$outputFile = tempnam(sys_get_temp_dir(), "clamdscanjob");
		$cmd = sprintf("sudo sh '%s/clamdscan-%s' >%s 2>&1 & echo $!",
		  $GLOBALS['OMV_CRONSCRIPTS_DIR'], $data['uuid'], $outputFile);
		OMVUtil::exec($cmd, $output, $res);
		if ($res !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Remember pid and content filename in session
		$pid = intval($output[0]);
		$_SESSION['execjob'][$pid] = array(
			"pid" => $pid,
			"outputFile" => $outputFile
		);
		return $pid;
	}
}
?>
