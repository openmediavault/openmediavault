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
namespace OMV\System\Net\NetworkInterfaceBackend;

/**
 * Implements the network interface backend for bridge devices.
 * @ingroup api
 */
class Bridge extends BackendAbstract {
	protected $regex = "/^br[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_BRIDGE;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}

	function enumerateSlaves() {
		if (FALSE === ($devs = $this->enumerate()))
			return FALSE;
		$result = [];
		foreach ($devs as $devk => $devv) {
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			$slaves = $object->getSlaves();
			if (FALSE === $slaves)
				continue;
			$result = array_merge($result, $slaves);
		}
		return $result;
	}

	function getImpl($args) {
		return new \OMV\System\Net\NetworkInterfaceBridge($args);
	}
}
