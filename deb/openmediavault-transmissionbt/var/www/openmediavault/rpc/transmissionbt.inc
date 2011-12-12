<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    Marcel Beck <marcel.beck@mbeck.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
 * @copyright Copyright (c) 2011 Marcel Beck
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
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

require_once("openmediavault/transmission.inc");

class TransmissionBTRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'},
					"pexenabled":{"type":"boolean"},
					"dhtenabled":{"type":"boolean"},
					"encryption":{"type":"integer","enum":[0,1,2]},
					"rpcurl":{"type":"string","format":"regex","pattern":"/^[a-z0-9]+$/i"},
					"rpcport":{"type":"integer","minimum":1024,"maximum":65535},
					"rpcauthenticationrequired":{"type":"boolean"},
					"rpcusername":{"type":"string"},
					"rpcpassword":{"type":"string"},
					"blocklistenabled":{"type":"boolean"},
					"blocklistsyncenabled":{"type":"boolean"},
					"blocklistsyncfrequency":{"type":"string","enum":'.'["hourly","daily","weekly","monthly"]},
					"blocklisturl":{"type":"string"},
					"lazy-bitfield-enabled":{"type":"boolean"},
					"umask":{"type":"integer"},
					"lpd-enabled":{"type":"boolean"},
					"scrape-paused-torrents-enabled":{"type":"boolean"},
					"utp-enabled":{"type":"boolean"},
					"message-level":{"type":"integer","enum":[0,1,2,3]},
					"cache-size-mb":{"type":"integer"}
				}
			}'),
			"setLocationsAndFiles" => array('{
				"type":"object",
				"properties":{
					"download-dir":{"type":"string"},
					"incomplete-dir":{"type":"string"},
					"incomplete-dir-enabled":{"type":"boolean"},
					"watch-dir":{"type":"string"},
					"watch-dir-enabled":{"type":"boolean"},
					"preallocation":{"type":"integer","enum":[0,1,2]},
					"rename-partial-files":{"type":"boolean"},
					"start-added-torrents":{"type":"boolean"},
					"trash-original-torrent-files":{"type":"boolean"}
				}
			}'),
			"setBandwidth" => array('{
				"type":"object",
				"properties":{
					"speed-limit-down":{"type":"integer","minimum":0},
					"speed-limit-down-enabled":{"type":"boolean"},
					"speed-limit-up":{"type":"integer","minimum":0},
					"speed-limit-up-enabled":{"type":"boolean"},
					"upload-slots-per-torrent":{"type":"integer"},
					"alt-speed-enabled":{"type":"boolean"},
					"alt-speed-up":{"type":"integer","minimum":0},
					"alt-speed-down":{"type":"integer","minimum":0}
				}
			}'),
			"setQueuing" => array('{
				"type":"object",
				"properties":{
					"queue-stalled-enabled":{"type":"boolean"},
					"queue-stalled-minutes":{"type":"integer","minimum":0},
					"download-queue-enabled":{"type":"boolean"},
					"download-queue-size":{"type":"integer","minimum":0},
					"seed-queue-enabled":{"type":"boolean"},
					"seed-queue-size":{"type":"integer","minimum":0}
				}
			}'),
			"setPeer" => array('{
				"type":"object",
				"properties":{
					"bind-address-ipv4":{"type":"string","format":"regex","pattern":"/^[0-9]{1,3}\\\.[0-9]{1,3}\\\.[0-9]{1,3}\\\.[0-9]{1,3}/"},
					"bind-address-ipv6":{"type":"string"},
					"peer-limit-global":{"type":"integer","minimum":0},
					"peer-limit-per-torrent":{"type":"integer","minimum":0},
					"peer-socket-tos":{"type":"string","enum":'.'["default","lowcost","throughput","lowdelay","reliability"]},
					"peer-port":{"type":"integer","minimum":1024,"maximum":65535},
					"peer-port-random-low":{"type":"integer","minimum":1024,"maximum":65535},
					"peer-port-random-high":{"type":"integer","minimum":1024,"maximum":65535},
					"peer-port-random-on-start":{"type":"boolean"},
					"port-forwarding-enabled":{"type":"boolean"}
				}
			}'),
			"setScheduling" => array('{
				"type":"object",
				"properties":{
					"alt-speed-time-enabled":{"type":"boolean"},
					"begin-hour":{"type":"integer","minimum":0,"maximum":23},
					"begin-minute":{"type":"integer","minimum":0,"maximum":59},
					"end-hour":{"type":"integer","minimum":0,"maximum":23},
					"end-minute":{"type":"integer","minimum":0,"maximum":59},
					"days-sunday":{"type":"boolean"},
					"days-monday":{"type":"boolean"},
					"days-tuesday":{"type":"boolean"},
					"days-wednesday":{"type":"boolean"},
					"days-thursday":{"type":"boolean"},
					"days-friday":{"type":"boolean"},
					"days-saturday":{"type":"boolean"},
					"idle-seeding-limit":{"type":"integer","minimum":0},
					"idle-seeding-limit-enabled":{"type":"boolean"},
					"ratio-limit":{"type":"number","minimum":0},
					"ratio-limit-enabled":{"type":"boolean"}
				}
			}'),
			"getList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}', // sortDir
				'{"type":"array","items":{"type":"string"}}' // types
			),
			"delete" => array('{
				"type":"object",
				"properties":{
					"id":{"type":"integer", "minimum":0},
					"deleteLocalData":{"type":"boolean"}
				}
			}'),
			"pause" => array(
				'{"type":"integer", "minimum":0}'
			),
			"resume" => array(
				'{"type":"integer", "minimum":0}'
			),
			"upload" => array(
				'{"type":"boolean"}'
			),
			"queueMove" => array('{
				"type":"object",
				"properties":{
					"id":{"type":"integer", "minimum":0},
					"action":{"type":"string","enum":'.'["top","up","down","bottom"]}
				}
			}'),
			"addUrl" => array('{
				"type":"object",
				"properties":{
					"url":{"type":"string"},
					"start_download":{"type":"boolean"}
				}
			}'),
		);
	}

	function getStatus() {
		$TransmissionBTModule = new TransmissionBTModule();
		return array($TransmissionBTModule->getStatus());
	}

	/**
	 * Get general settings object
	 */
	function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/transmissionbt";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['pexenabled'] = boolval($object['pexenabled']);
		$object['dhtenabled'] = boolval($object['dhtenabled']);
		$object['encryption'] = intval($object['encryption']);
		$object['rpcport'] = intval($object['rpcport']);
		$object['rpcurl'] = trim($object['rpcurl'], '/');
		$object['rpcauthenticationrequired'] = boolval($object['rpcauthenticationrequired']);
		$object['blocklistenabled'] = boolval($object['blocklistenabled']);
		$object['blocklistsyncenabled'] = boolval($object['blocklistsyncenabled']);
		$object['lazy-bitfield-enabled'] = boolval($object['lazy-bitfield-enabled']);
		$object['umask'] = intval($object['umask']);
		$object['lpd-enabled'] = boolval($object['lpd-enabled']);
		$object['scrape-paused-torrents-enabled'] = boolval($object['scrape-paused-torrents-enabled']);
		$object['utp-enabled'] = boolval($object['utp-enabled']);
		$object['message-level'] = intval($object['message-level']);
		$object['cache-size-mb'] = intval($object['cache-size-mb']);
		return $object;
	}

	/**
	 * Set general settings object
	 */
	function setSettings($data) {
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
		// Get existing configuration object
		$xpath = "//services/transmissionbt";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Prepare configuration data
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"sharedfolderref" => $data['sharedfolderref'],
			"pexenabled" => array_boolval($data, 'pexenabled'),
			"dhtenabled" => array_boolval($data, 'dhtenabled'),
			"encryption" => $data['encryption'],
			"rpcurl" => '/' . $data['rpcurl'] . '/',
			"rpcport" => $data['rpcport'],
			"rpcauthenticationrequired" => array_boolval($data,'rpcauthenticationrequired'),
			"rpcusername" => $data['rpcusername'],
			"rpcpassword" => $data['rpcpassword'],
			"blocklistenabled" => array_boolval($data, 'blocklistenabled'),
			"blocklistsyncenabled" => array_boolval($data,'blocklistsyncenabled'),
			"blocklistsyncfrequency" => $data['blocklistsyncfrequency'],
			"blocklisturl" => $data['blocklisturl'],
			"modules" => isset($oldObject['modules']) ? $oldObject['modules'] : array(),
			"lazy-bitfield-enabled" => array_boolval($data,'lazy-bitfield-enabled'),
			"umask" => $data['umask'],
			"lpd-enabled" => array_boolval($data,'lpd-enabled'),
			"scrape-paused-torrents-enabled" => array_boolval($data,'scrape-paused-torrents-enabled'),
			"utp-enabled" => array_boolval($data,'utp-enabled'),
			"message-level" => $data['message-level'],
			"cache-size-mb" => $data['cache-size-mb'],
		);
		// Set configuration object
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.transmissionbt", $object);
	}

	/**
	 * Get locations and files settings object
	 */
	function getLocationsAndFiles() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/transmissionbt/modules/locationsandfiles";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		// Locations
		$object['incomplete-dir-enabled'] = boolval($object['incomplete-dir-enabled']);
		$object['watch-dir-enabled'] = boolval($object['watch-dir-enabled']);
		// Files
		$object['preallocation'] = intval($object['preallocation']);
		$object['rename-partial-files'] = boolval($object['rename-partial-files']);
		$object['start-added-torrents'] = boolval($object['start-added-torrents']);
		$object['trash-original-torrent-files'] = boolval($object['trash-original-torrent-files']);
		return $object;
	}

	/**
	 * Set locations and files settings object
	 */
	function setLocationsAndFiles($data) {
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
			// Locations
			"download-dir" => $data['download-dir'],
			"incomplete-dir" => $data['incomplete-dir'],
			"incomplete-dir-enabled" => $data['incomplete-dir-enabled'],
			"watch-dir" => $data['watch-dir'],
			"watch-dir-enabled" => $data['watch-dir-enabled'],
			// Files
			"preallocation" => $data['preallocation'],
			"rename-partial-files" => array_boolval($data, 'rename-partial-files'),
			"start-added-torrents" => array_boolval($data, 'start-added-torrents'),
			"trash-original-torrent-files" => array_boolval($data, 'trash-original-torrent-files'),
		);
		// Set configuration object
		$xpath = "//services/transmissionbt/modules/locationsandfiles";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.transmissionbt", $object);
	}

	/**
	 * Get bandwidth settings object
	 */
	function getBandwidth() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/transmissionbt/modules/bandwidth";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		// Speed
		$object['speed-limit-down'] = intval($object['speed-limit-down']);
		$object['speed-limit-down-enabled'] = boolval($object['speed-limit-down-enabled']);
		$object['speed-limit-up'] = intval($object['speed-limit-up']);
		$object['speed-limit-up-enabled'] = boolval($object['speed-limit-up-enabled']);
		$object['upload-slots-per-torrent'] = intval($object['upload-slots-per-torrent']);
		// Turtle Mode
		$object['alt-speed-enabled'] = boolval($object['alt-speed-enabled']);
		$object['alt-speed-up'] = intval($object['alt-speed-up']);
		$object['alt-speed-down'] = intval($object['alt-speed-down']);

		return $object;
	}

	/**
	 * Set bandwidth settings object
	 */
	function setBandwidth($data) {
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
			// Speed
			"speed-limit-down" => $data['speed-limit-down'],
			"speed-limit-down-enabled" => array_boolval($data, 'speed-limit-down-enabled'),
			"speed-limit-up" => $data['speed-limit-up'],
			"speed-limit-up-enabled" => array_boolval($data, 'speed-limit-up-enabled'),
			"upload-slots-per-torrent" => $data['upload-slots-per-torrent'],
			// Turtle Mode
			"alt-speed-enabled" => array_boolval($data, 'alt-speed-enabled'),
			"alt-speed-up" => $data['alt-speed-up'],
			"alt-speed-down" => $data['alt-speed-down'],
		);
		// Set configuration object
		$xpath = "//services/transmissionbt/modules/bandwidth";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.transmissionbt", $object);
	}

	/**
	 * Get queuing settings object
	 */
	function getQueuing() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/transmissionbt/modules/queuing";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		// General
		$object['queue-stalled-enabled'] = boolval($object['queue-stalled-enabled']);
		$object['queue-stalled-minutes'] = intval($object['queue-stalled-minutes']);
		// Download
		$object['download-queue-enabled'] = boolval($object['download-queue-enabled']);
		$object['download-queue-size'] = intval($object['download-queue-size']);
		// Seed
		$object['seed-queue-enabled'] = boolval($object['seed-queue-enabled']);
		$object['seed-queue-size'] = intval($object['seed-queue-size']);

		return $object;
	}

	/**
	 * Set queueing settings object
	 */
	function setQueuing($data) {
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
			// General
			"queue-stalled-enabled" => array_boolval($data, 'queue-stalled-enabled'),
			"queue-stalled-minutes" => $data['queue-stalled-minutes'],
			// Download
			"download-queue-enabled" => array_boolval($data, 'download-queue-enabled'),
			"download-queue-size" => $data['download-queue-size'],
			// Seed
			"seed-queue-enabled" => array_boolval($data, 'seed-queue-enabled'),
			"seed-queue-size" => $data['seed-queue-size'],
		);
		// Set configuration object
		$xpath = "//services/transmissionbt/modules/queuing";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.transmissionbt", $object);
	}

	/**
	 * Get peer settings object
	 */
	function getPeer() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/transmissionbt/modules/peer";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		// Peers
		// Limits
		$object['peer-limit-global'] = intval($object['peer-limit-global']);
		$object['peer-limit-per-torrent'] = intval($object['peer-limit-per-torrent']);
		// Peer Ports
		$object['peer-port'] = intval($object['peer-port']);
		$object['peer-port-random-high'] = intval($object['peer-port-random-high']);
		$object['peer-port-random-low'] = intval($object['peer-port-random-low']);
		$object['peer-port-random-on-start'] = boolval($object['peer-port-random-on-start']);
		$object['port-forwarding-enabled'] = boolval($object['port-forwarding-enabled']);
		return $object;
	}

	/**
	 * Set peer settings object
	 */
	function setPeer($data) {
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
			// Peers
			// Bindings
			"bind-address-ipv4" => $data['bind-address-ipv4'],
			"bind-address-ipv6" => $data['bind-address-ipv6'],
			// Limits
			"peer-limit-global" => $data['peer-limit-global'],
			"peer-limit-per-torrent" => $data['peer-limit-per-torrent'],
			"peer-socket-tos" => $data['peer-socket-tos'],
			// Peer Ports
			"peer-port" => $data['peer-port'],
			"peer-port-random-high" => $data['peer-port-random-high'],
			"peer-port-random-low" => $data['peer-port-random-low'],
			"peer-port-random-on-start" => array_boolval($data, 'peer-port-random-on-start'),
			"port-forwarding-enabled" => array_boolval($data, 'port-forwarding-enabled'),
		);
		// Set configuration object
		$xpath = "//services/transmissionbt/modules/peer";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.transmissionbt", $object);
	}

	/**
	 * Get scheduling settings object
	 */
	function getScheduling() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/transmissionbt/modules/scheduling";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		// Scheduling
		$object['alt-speed-time-enabled'] = boolval($object['alt-speed-time-enabled']);
		$object['alt-speed-time-begin'] = intval($object['alt-speed-time-begin']);
		$object['begin-hour'] = intval($object['alt-speed-time-begin'] / 60);
		$object['begin-minute'] = ($object['alt-speed-time-begin'] % 60);
		$object['alt-speed-time-end'] = intval($object['alt-speed-time-end']);
		$object['end-hour'] = intval($object['alt-speed-time-end'] / 60);
		$object['end-minute'] = ($object['alt-speed-time-end'] % 60);
		$object['days-sunday'] = false;
		if (($object['alt-speed-time-day'] & 1) === 1) {
			$object['days-sunday'] = true;
		}
		$object['days-monday'] = false;
		if (($object['alt-speed-time-day'] & 2) === 2) {
			$object['days-monday'] = true;
		}
		$object['days-tuesday'] = false;
		if (($object['alt-speed-time-day'] & 4) === 4) {
			$object['days-tuesday'] = true;
		}
		$object['days-wednesday'] = false;
		if (($object['alt-speed-time-day'] & 8) === 8) {
			$object['days-wednesday'] = true;
		}
		$object['days-thursday'] = false;
		if (($object['alt-speed-time-day'] & 16) === 16) {
			$object['days-thursday'] = true;
		}
		$object['days-friday'] = false;
		if (($object['alt-speed-time-day'] & 32) === 32) {
			$object['days-friday'] = true;
		}
		$object['days-saturday'] = false;
		if (($object['alt-speed-time-day'] & 64) === 64) {
			$object['days-saturday'] = true;
		}
		// Idle
		$object['idle-seeding-limit'] = intval($object['idle-seeding-limit']);
		$object['idle-seeding-limit-enabled'] = boolval($object['idle-seeding-limit-enabled']);
		// Ratio
		$object['ratio-limit'] = floatval($object['ratio-limit']);
		$object['ratio-limit-enabled'] = boolval($object['ratio-limit-enabled']);
		return $object;
	}

	/**
	 * Set scheduling settings object
	 */
	function setScheduling($data) {
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
		$data['alt-speed-time-day'] = 0;
		if (array_boolval($data, 'days-sunday') === true) {
			$data['alt-speed-time-day'] += 1;
		}
		if (array_boolval($data, 'days-monday') === true) {
			$data['alt-speed-time-day'] += 2;
		}
		if (array_boolval($data, 'days-tuesday') === true) {
			$data['alt-speed-time-day'] += 4;
		}
		if (array_boolval($data, 'days-wednesday') === true) {
			$data['alt-speed-time-day'] += 8;
		}
		if (array_boolval($data, 'days-thursday') === true) {
			$data['alt-speed-time-day'] += 16;
		}
		if (array_boolval($data, 'days-friday') === true) {
			$data['alt-speed-time-day'] += 32;
		}
		if (array_boolval($data, 'days-saturday') === true) {
			$data['alt-speed-time-day'] += 64;
		}
		// Prepare configuration data
		$object = array(
			'alt-speed-time-enabled' => array_boolval($data, 'alt-speed-time-enabled'),
			'alt-speed-time-begin' => intval(($data['begin-hour'] * 60) + $data['begin-minute']),
			'alt-speed-time-end' => intval(($data['end-hour'] * 60) + $data['end-minute']),
			'alt-speed-time-day' => $data['alt-speed-time-day'],
			// Idle
			'idle-seeding-limit' => $data['idle-seeding-limit'],
			'idle-seeding-limit-enabled' => array_boolval($data, 'idle-seeding-limit-enabled'),
			// Ratio
			'ratio-limit' => $data['ratio-limit'],
			'ratio-limit-enabled' => array_boolval($data, 'ratio-limit-enabled'),
		);
		// Set configuration object
		$xpath = "//services/transmissionbt/modules/scheduling";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.transmissionbt", $object);
	}

	/**
	 * Get list of torrents
	 */
	public function getList($start, $count, $sortField, $sortDir, $types) {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());

		$transmissionbt_rpc = $this->factoryTransmission();

		$torrents = $transmissionbt_rpc->get(array(), array(
															'id',
															'name',
															'status',
															'totalSize',
															'haveValid',
															'percentDone',
															'eta',
															'peersConnected',
															'peersSendingToUs',
															'rateDownload',
															'rateUpload',
															'addedDate',
															'doneDate',
															'uploadRatio',
															'queuePosition',
															)
		);

		$torrents = $torrents['arguments']['torrents'];

		// Filter result
		return $this->applyFilter($torrents, $start, $count, $sortField, $sortDir);

	}

	/**
	* Process uploaded torrent file
	*/
	function upload() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Is file successfully upload?
		if (!is_uploaded_file($_FILES['file']['tmp_name'])) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Invalid upload request");
		}
		//$this->validateParams(__METHOD__, func_get_args());

		$transmissionbt_rpc = $this->factoryTransmission();

		$file_path = sys_get_temp_dir() . '/' . $_FILES['file']['name'];

		move_uploaded_file($_FILES['file']['tmp_name'], $file_path);

		//$start_download = !array_boolval($data, 'start-download');

		$torrent = $transmissionbt_rpc->add_file($file_path, ''/*, $start_download*/);
		unlink($file_path);
		return $torrent;
	}

	function addUrl($data) {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		$this->validateParams(__METHOD__, func_get_args());

		$transmissionbt_rpc = $this->factoryTransmission();

		$url = $data['url'];
		$paused = !array_boolval($data, 'start_download');

		$result = $transmissionbt_rpc->add($url, '', array('paused' => $paused));

		if (isset($result['result']))
		{
			if ($result['result'] == 'success')
			{
				return 'success';
			}
			else
			{
				throw new Exception($result['result']);
			}
		}
		throw new Exception('Something went really wrong');
	}

	/**
	* Delete specific torrent
	* Optional delete local data
	*/
	public function delete($data) {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());

		$transmissionbt_rpc = $this->factoryTransmission();

		$transmissionbt_rpc->remove($data['id'], array_boolval($data, 'deleteLocalData'));

		return 'success';
	}

	/**
	* Resume specific torrent
	*/
	public function resume($id) {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());

		$transmissionbt_rpc = $this->factoryTransmission();

		$transmissionbt_rpc->start($id);

		return 'success';
	}

	/**
	* Pause specific torrent
	*/
	public function pause($id) {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());

		$transmissionbt_rpc = $this->factoryTransmission();

		$transmissionbt_rpc->stop($id);

		return 'success';
	}

	/**
	 * Queue move specific torrent
	 *
	 * @throws OMVException
	 * @param $id
	 * @return string
	 */
	public function queueMove($data) {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration data

		$transmissionbt_rpc = $this->factoryTransmission();

		switch ($data['action'])
		{
			case 'top':
				$transmissionbt_rpc->queueMoveTop($data['id']);
				break;
			case 'up':
				$transmissionbt_rpc->queueMoveUp($data['id']);
				break;
			case 'down':
				$transmissionbt_rpc->queueMoveDown($data['id']);
				break;
			case 'bottom':
				$transmissionbt_rpc->queueMoveBottom($data['id']);
				break;
			default:
				throw new OMVException('Unknown Action: ' . $data['action']);
				break;
		}

		return 'success';
	}

	private function factoryTransmission()
	{
		global $xmlConfig;

		$xpath = "//services/transmissionbt";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}

		$transmissionbt_rpc = new Transmission();

		$transmissionbt_rpc->url = 'http://localhost:'.$object['rpcport'].$object['rpcurl'].'rpc';

		$transmissionbt_rpc->return_as_array = true;
		// Set authentication when needed
		if (array_boolval($object, 'rpcauthenticationrequired') === true)
		{
			$transmissionbt_rpc->username = $object['rpcusername'];
			$transmissionbt_rpc->password = $object['rpcpassword'];
		}

		return $transmissionbt_rpc;
	}
}
?>
