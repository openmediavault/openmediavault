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
namespace OMV;

require_once("openmediavault/globals.inc");

/**
 * This class provides a simple interface to get product information.
 * @ingroup api
 */
class ProductInfo {
	private $data = array();

	public function __construct() {
		$xml = simplexml_load_file($GLOBALS['OMV_PRODUCTINFO_FILE']);
		if (FALSE === $xml) {
			throw new \OMV\Exception("Failed to load product information: %s",
			  implode("\n", libxml_get_errors()));
		}
		$this->data = $xml;
	}

	/**
	 * Get the product name.
	 * @return The product name string, otherwise FALSE.
	 */
	public function getName() {
		return $this->data->name;
	}

	/**
	 * Get the product version.
	 * @return The product version as string, otherwise FALSE.
	 */
	public function getVersion() {
		$cmdArgs = array();
		$cmdArgs[] = "-W";
		$cmdArgs[] = "-f='\${Version}'";
		$cmdArgs[] = $this->getPackageName();
		// Execute the command.
		$cmd = new \OMV\System\Process("dpkg-query", $cmdArgs);
		return $cmd->execute();
	}

	/**
	 * Get the product version/release name.
	 * @return The release name as string, otherwise FALSE.
	 */
	public function getVersionName() {
		return $this->data->versionname;
	}

	/**
	 * Get the URL to the product homepage.
	 * @return The URL, otherwise FALSE.
	 */
	public function getURL() {
		return $this->data->url;
	}

	/**
	 * Get the copyright text.
	 * @return The copyright text, otherwise FALSE.
	 */
	public function getCopyright() {
		return $this->data->copyright;
	}

	/**
	 * Get the Debian package name.
	 * @return The package name, otherwise FALSE.
	 */
	public function getPackageName() {
		return $this->data->packagename;
	}
}
