<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
namespace OMV\System\Network;

/**
 * @ingroup api
 */
class Dns {
	/**
	 * Get the systems host name.
	 * @return The host name.
	 */
	final public static function getHostname() {
		$cmd = new \OMV\System\Process("hostname", $options);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);
		return $output[0];
	}

	/**
	 * Get the systems fully qualified domain name (FQDN).
	 * @return The fully qualified domain name.
	 */
	final public static function getFqdn() {
		$cmd = new \OMV\System\Process("hostname", "--fqdn");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);
		return $output[0];
	}

	/**
	 * Get the systems domain name.
	 * @return The domain name.
	 */
	final public static function getDomain() {
		$cmd = new \OMV\System\Process("hostname", "--domain");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);
		return $output[0];
	}
}
