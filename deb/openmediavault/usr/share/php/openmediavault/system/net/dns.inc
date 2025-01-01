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
 * @ingroup api
 */
class Dns {
	/**
	 * Get the system's host name.
	 * @return string The system's host name.
	 */
	final public static function getHostname() {
		$cmd = new \OMV\System\Process("hostname");
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return $output[0];
	}

	/**
	 * Get the system's short host name. This is the host name cut at
	 * the first dot.
	 * @return string The system's short host name.
	 */
	final public static function getShortHostname() {
		$cmd = new \OMV\System\Process("hostname", ["--short"]);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return $output[0];
	}

	/**
	 * Get the system's fully qualified domain name (FQDN), e.g.
	 * 'leto.example.com'. It is usually the hostname followed by
     * the DNS domain name (the part after the first dot).
	 * @return string The system's fully qualified domain name.
	 */
	final public static function getFqdn() {
		$cmd = new \OMV\System\Process("hostname", ["--fqdn"]);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return $output[0];
	}

	/**
	 * Get the name of the system's DNS domain.
	 * @return string The system's DNS domain name.
	 */
	final public static function getDomain() {
		$cmd = new \OMV\System\Process("hostname", ["--domain"]);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return $output[0];
	}
}
