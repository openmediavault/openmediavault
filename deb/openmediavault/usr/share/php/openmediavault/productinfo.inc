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
namespace OMV;

/**
 * This class provides a simple interface to get product information.
 * @ingroup api
 */
class ProductInfo {
	private $data = [];

	/**
	 * Constructor
	 */
	public function __construct() {
		$xml = simplexml_load_file(\OMV\Environment::get(
		  "OMV_PRODUCTINFO_FILE"));
		if (FALSE === $xml) {
			throw new \OMV\Exception("Failed to load product information: %s",
			  implode("\n", libxml_get_errors()));
		}
		$this->data = $xml;
	}

	/**
	 * Get the product name.
	 * @return The product name string.
	 */
	public function getName() {
		return $this->data->name;
	}

	/**
	 * Get the product version.
	 * @return The product version as string.
	 */
	public function getVersion() {
		$cmdArgs = [];
		$cmdArgs[] = "-W";
		$cmdArgs[] = "-f='\${Version}'";
		$cmdArgs[] = $this->getPackageName();
		$cmd = new \OMV\System\Process("dpkg-query", $cmdArgs);
		$cmd->setRedirect2to1();
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
	 * @return The URL of the product homepage.
	 */
	public function getURL() {
		return $this->data->url;
	}

	/**
	 * Get the copyright text.
	 * @return The copyright text.
	 */
	public function getCopyright() {
		return $this->data->copyright;
	}

	/**
	 * Get the Debian package name prefix.
	 * @return The package name prefix.
	 */
	public function getPackageName() {
		return $this->data->packagename;
	}
}
