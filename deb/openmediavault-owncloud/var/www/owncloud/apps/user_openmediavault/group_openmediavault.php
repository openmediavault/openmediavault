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
require_once("openmediavault/rpc.inc");

class OC_Group_OpenMediaVault extends OC_Group_Backend {
	private $context = NULL;

	public function __construct() {
		$this->context = array(
			"username" => "admin",
			"role" => OMV_ROLE_ADMINISTRATOR
		);
	}

	/**
	 * @brief Get all groups a user belongs to
	 * @param string $uid Name of the user
	 * @return array with group names
	 *
	 * This function fetches all groups a user belongs to. It does not check
	 * if the user exists at all.
	 */
	public function getUserGroups($uid) {
		$groups = array();
		try {
			$user = OMVRpc::exec("UserMgmt", "getUser", array(
			  	  "name" => strtolower($uid)
			  ), $this->context, OMV_RPC_MODE_REMOTE);
			$groups = $user['groups'];
		} catch(Exception $e) {
			OC_Log::write("OC_Group_OpenMediaVault", sprintf(
			  "Failed to get user '%s' (code=%d, message=%s)", $uid,
			  $e->getCode(), $e->getMessage()), OC_Log::ERROR);
		}
		return $groups;
	}

	/**
	 * @brief get a list of all groups
	 * @param string $search
	 * @param int $limit
	 * @param int $offset
	 * @return array with group names
	 *
	 * Returns a list with all groups
	 */
	public function getGroups($search = '', $limit = NULL, $offset = 0) {
		$result = array();
		try {
			$groups = OMVRpc::exec("UserMgmt", "getGroupList", array(
			      "start" => intval($offset),
			      "limit" => !is_null($limit) ? intval($limit) : NULL,
			      "sortfield" => !empty($search) ? $search : NULL,
			      "sortdir" => NULL
			  ), $this->context, OMV_RPC_MODE_REMOTE);
			foreach($groups['data'] as $groupk => $groupv) {
				$result[] = $groupv['name'];
			}
		} catch(Exception $e) {
			OC_Log::write("OC_User_OpenMediaVault", sprintf(
			  "Failed to get groups (code=%d, message=%s)", $e->getCode(),
			  $e->getMessage()), OC_Log::ERROR);
		}
		return $result;
	}

	/**
	 * check if a group exists
	 * @param string $gid
	 * @return bool
	 */
	public function groupExists($gid) {
		return in_array($gid, $this->getGroups());
	}

	/**
	 * @brief get a list of all users in a group
	 * @param string $gid
	 * @param string $search
	 * @param int $limit
	 * @param int $offset
	 * @return array with user ids
	 */
	public function usersInGroup($gid, $search = '', $limit = NULL, $offset = 0) {
		$users = array();
		try {
			$group = OMVRpc::exec("UserMgmt", "getGroup", array(
			  	  "name" => strtolower($uid)
			  ), $this->context, OMV_RPC_MODE_REMOTE);
			$users = $group['members'];
		} catch(Exception $e) {
			OC_Log::write("OC_Group_OpenMediaVault", sprintf("Failed to ".
			  "get users in group '%s' (code=%d, message=%s)", $gid,
			  $e->getCode(), $e->getMessage()), OC_Log::ERROR);
		}
		return $users;
	}

	/**
	 * @brief Try to create a new group
	 * @param string $gid The name of the group to create
	 * @return bool
	 *
	 * Tries to create a new group. If the group name already exists, false will
	 * be returned.
	 */
	public function createGroup($gid) {
		OC_Log::write("OC_Group_OpenMediaVault", "Not possible to create a ".
		  "group from web frontend using OpenMediaVault user backend",
		  OC_Log::ERROR);
		return FALSE;
	}

	/**
	 * @brief delete a group
	 * @param string $gid gid of the group to delete
	 * @return bool
	 *
	 * Deletes a group and removes it from the group_user-table
	 */
	public function deleteGroup($gid) {
		OC_Log::write("OC_Group_OpenMediaVault", "Not possible to delete a ".
		  "group from web frontend using OpenMediaVault user backend",
		  OC_Log::ERROR);
		return FALSE;
	}

	/**
	 * @brief Add a user to a group
	 * @param string $uid Name of the user to add to group
	 * @param string $gid Name of the group in which add the user
	 * @return bool
	 *
	 * Adds a user to a group.
	 */
	public function addToGroup($uid, $gid) {
		OC_Log::write("OC_Group_OpenMediaVault", "Not possible to add a ".
		  "user to a group from web frontend using OpenMediaVault user ".
		  "backend", OC_Log::ERROR);
		return FALSE;
	}

	/**
	 * @brief Removes a user from a group
	 * @param string $uid Name of the user to remove from group
	 * @param string $gid Name of the group from which remove the user
	 * @return bool
	 *
	 * removes the user from a group.
	 */
	public function removeFromGroup($uid, $gid) {
		OC_Log::write("OC_Group_OpenMediaVault", "Not possible to remove a ".
		  "user from a group from web frontend using OpenMediaVault user ".
		  "backend", OC_Log::ERROR);
		return FALSE;
	}
}
?>
