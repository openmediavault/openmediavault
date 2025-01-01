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

class Quota extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Quota";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("getByTypeName");
		$this->registerMethod("setByTypeName");
		$this->registerMethod("delete");
	}

    /**
	 * Get the quota for the given filesystem.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the filesystem.
	 * @param context The context of the caller.
	 * @return An array containing the requested quota configuration.
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get filesystem details.
		\OMV\System\Filesystem\Filesystem::assertGetImpl($params['uuid']);
		$fs = \OMV\System\Filesystem\Filesystem::getImpl($params['uuid']);
		$devicefile = $fs->getDeviceFile();
		// Does any quota configuration exist for the given filesystem?
		$object = NULL;
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "fsuuid",
			"arg1" => $params['uuid']
		];
		$db = \OMV\Config\Database::getInstance();
		if ($db->exists("conf.system.filesystem.quota", $filter)) {
			// Load the existing configuration object.
			// Due the fact that a quota configuration only exists once per
			// filesystem we can simply use the first found object.
			$object = $db->getByFilter("conf.system.filesystem.quota",
			  $filter, 1);
		}
		// Prepare result. Iterate over all system users and groups and get
		// their quota configuration for the given filesystem.
		$result = [];
		// Get non-system users
		$users = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateUsers", NULL,
		  $context);
		foreach ($users as $userk => $userv) {
			// Set default values.
			$resObj = [
				"type" => "user",
				"name" => $userv['name'],
				"bused" => "0 B",
				"bhardlimit" => 0,
				"bunit" => "MiB"
			];
			// Check if there is any configured quota for the given user.
			if (is_object($object)) {
				foreach ($object->get("usrquota") as $usrquotav) {
					if ($userv['name'] !== $usrquotav['name'])
						continue;
					// Get user quotas and get the number of used disk space
					// if available. Note, 'blocks' are given in KiB.
					$sysUser = new \OMV\System\User($userv['name']);
					if (FALSE !== ($quotas = $sysUser->getQuotas())) {
						if (array_key_exists($devicefile, $quotas)) {
							$resObj['bused'] = binary_format(
							  $quotas[$devicefile]['blocks'],
							  [ "fromPrefix" => "KiB" ]);
						}
					}
					// Convert the quota to a human readable unit. Note, the
					// values are stored a KiB in the configuration.
					$bhardlimit = binary_format($usrquotav['bhardlimit'], [
						  "fromPrefix" => "KiB",
						  "maxPrefix" => "TiB",
						  "indexed" => TRUE
					  ]);
					$resObj['bhardlimit'] = $bhardlimit['value'];
					$resObj['bunit'] = $bhardlimit['unit'];
				}
			}
			$result[] = $resObj;
		}
		// Get non-system groups
		$groups = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateGroups", NULL,
		  $context);
		foreach ($groups as $groupk => $groupv) {
			// Set default values.
			$resObj = [
				"type" => "group",
				"name" => $groupv['name'],
				"bused" => "0 B",
				"bhardlimit" => 0,
				"bunit" => "MiB"
			];
			// Check if there is any configured quota for the given group.
			if (is_object($object)) {
				foreach ($object->get("grpquota") as $grpquotav) {
					if ($groupv['name'] !== $grpquotav['name'])
						continue;
					// Get user quotas and get the number of used disk space
					// if available. Note, 'blocks' are given in KiB.
					$sysGroup = new \OMV\System\Group($groupv['name']);
					if (FALSE !== ($quotas = $sysGroup->getQuotas())) {
						if (array_key_exists($devicefile, $quotas)) {
							$resObj['bused'] = binary_format(
							  $quotas[$devicefile]['blocks'],
							  [ "fromPrefix" => "KiB" ]);
						}
					}
					// Convert the quota to a human readable unit. Note, the
					// values are stored a KiB in the configuration.
					$bhardlimit = binary_format($grpquotav['bhardlimit'], [
						  "fromPrefix" => "KiB",
						  "maxPrefix" => "TiB",
						  "indexed" => TRUE
					  ]);
					$resObj['bhardlimit'] = $bhardlimit['value'];
					$resObj['bunit'] = $bhardlimit['unit'];
				}
			}
			$result[] = $resObj;
		}
		return $result;
	}

    /**
	 * Set the quota for the given filesystem.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the filesystem.
	 *   \em quota An array of quotas to set.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.quota.set");
		// Build the user/group quota configuration.
		$quotas = [
			"usrquota" => [],
			"grpquota" => []
		];
		foreach ($params['quota'] as $paramk => $paramv) {
			$quotaObj = [
				"name" => $paramv['name'],
				"bsoftlimit" => 0,
				// Store as string because of the 32bit integer limit of
				// PHP on 32bit systems.
				"bhardlimit" => binary_convert($paramv['bhardlimit'],
				  $paramv['bunit'], "KiB"),
				"isoftlimit" => 0,
				"ihardlimit" => 0
			];
			switch ($paramv['type']) {
			case "user":
				$quotas['usrquota'][] = $quotaObj;
				break;
			case "group":
				$quotas['grpquota'][] = $quotaObj;
				break;
			}
		}
		// Prepare the configuration object.
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "fsuuid",
			"arg1" => $params['uuid']
		];
		$db = \OMV\Config\Database::getInstance();
		if ($db->exists("conf.system.filesystem.quota", $filter)) {
			// Load the existing configuration objects.
			// Due the fact that a quota configuration only exists once per
			// filesystem we can simply use the first found object.
			$object = $db->getByFilter("conf.system.filesystem.quota",
			  $filter, 1);
			// Reset the existing quotas.
			$object->set("usrquota", []);
			$object->set("grpquota", []);
		} else {
			// Create a new configuration object.
			$object = new \OMV\Config\ConfigObject(
			  "conf.system.filesystem.quota");
			$object->setNew();
			$object->set("fsuuid", $params['uuid']);
		}
		$object->setAssoc($quotas);
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get the quota for the given filesystem and type/name.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the filesystem.
	 *   \em name The name of the user/group.
	 *   \em type The type, e.g. 'user' or 'group'.
	 * @param context The context of the caller.
	 * @return An object containing the fields \em uuid, \em type,
	 *   \em name, \em bhardlimit and \em bunit.
	 */
	function getByTypeName($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.quota.getByTypeName");
		// Get the quota configuration object.
		$quota = $this->callMethod("get", [
			"uuid" => $params['uuid']
		], $this->getAdminContext());
		// Try to find the given quota for type/name.
		$result = NULL;
		foreach($quota as $quotak => $quotav) {
			if (($quotav['type'] == $params['type']) &&
					($quotav['name'] == $params['name'])) {
				$result = $quotav;
				$result['uuid'] = $params['uuid'];
				break;
			}
		}
		if (is_null($result)) {
			throw new \OMV\Exception("Failed to get quota.");
		}
		return $result;
	}

	/**
	 * Set the quota for the given filesystem and type/name.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the filesystem.
	 *   \em name The name of the user/group.
	 *   \em type The type, e.g. 'user' or 'group'.
	 *   \em bhardlimit The hard limit.
	 *   \em bunit The unit, e.g. 'B', 'MiB' or 'PiB'.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setByTypeName($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.quota.setByTypeName");
		// Get the quota configuration object.
		$quota = $this->callMethod("get", [
			"uuid" => $params['uuid']
		], $this->getAdminContext());
		// Update the given data.
		foreach($quota as $quotak => &$quotav) {
			if (($quotav['type'] == $params['type']) &&
					($quotav['name'] == $params['name'])) {
				$quotav['type'] = $params['type'];
				$quotav['name'] = $params['name'];
				$quotav['bhardlimit'] = $params['bhardlimit'];
				$quotav['bunit'] = $params['bunit'];
				break;
			}
		}
		// Set the quota configuration.
		return $this->callMethod("set", [
			"uuid" => $params['uuid'],
			"quota" => $quota
		], $this->getAdminContext());
	}

	/**
	 * Delete a filesystem quota configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	function delete($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.filesystem.quota", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}
}
