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
namespace OMV\System\Net;

/**
 * This class provides a interface to handle Linux VLAN network
 * interfaces.
 * @ingroup api
 */
class NetworkInterfaceVlan extends NetworkInterface {
	public function getType() {
		return "vlan";
	}

	/**
	* Get VLAN ID of the network interface.
	* @return The VLAN id, otherwise FALSE.
	*/
	public function getVlanId() {
		$regex = "/^(\S+\d+)\.(\d+)$/i";
		if (1 !== preg_match($regex, $this->getDeviceName(), $matches))
			return FALSE;
		return !empty($matches[2]) ? intval($matches[2]) : FALSE;
	}

	/**
	 * Delete the network interface.
	 * @throw \OMV\ExecException
	 */
	public function delete() {
		parent::delete();
		$cmdArgs = [];
		$cmdArgs[] = "link";
		$cmdArgs[] = "del";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}
}
