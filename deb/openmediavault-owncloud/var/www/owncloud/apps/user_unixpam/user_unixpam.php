<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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

class OC_User_UnixPAM extends OC_User_Backend {
	/**
	 * @brief Check if the password is correct
	 * @param $uid The username
	 * @param $password The password
	 * @returns true/false
	 *
	 * Check if the password is correct without logging in the user
	 */
	public function checkPassword($uid, $password) {
		return pam_auth(strtolower($uid), $password);
	}

	/**
	 * @brief check if a user exists
	 * @param string $uid the username
	 * @return boolean
	 */
	public function userExists($uid) {
		$userinfo = posix_getpwnam(strtolower($uid));
		return is_array($userinfo);
	}

	/**
	 * @brief delete a user
	 * @param $uid The username of the user to delete
	 * @returns true/false
	 *
	 * Deletes a user
	 */
	public function deleteUser($uid) {
		OC_Log::write("OC_USER_UNIXPAM", "Not possible to delete users from ".
		  "web frontend using Unix PAM user backend", OC_Log::ERROR);
		return false;
	}

	public function setPassword($uid, $password) {
		OC_Log::write("OC_USER_UNIXPAM", "Not possible to change password ".
		  "for users from web frontend using Unix PAM user backend",
		  OC_Log::ERROR);
		return false;
	}
}
?>
