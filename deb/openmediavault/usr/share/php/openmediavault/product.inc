<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/util.inc");

class OMVProduct extends OMVObject {
	private $info = array();

	/**
	 * Get the product informations, e.g. name, version and URL.
	 * @return An array containing the product informations.
	 * array(
	 *   name => xxx,
	 *   version => xxx,
	 *   versionname => xxx,
	 *   url => xxx,
	 *   copyright => xxx,
	 *   packagename => xxx
	 * )
	 */
	private function getData() {
		if (!empty($this->info))
			return TRUE;
		$engine = new OMVConfigEngine($GLOBALS['OMV_PRODUCTINFO_FILE']);
		if (FALSE === $engine->load()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to load product information (%s): %s",
			  $GLOBALS['OMV_PRODUCTINFO_FILE'], $xmlConfig->getError()));
		}
		$object = $engine->get("/*");
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Failed to get product information");
		}
		$this->info = $object;
		return TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->info = array();
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the product name.
	 * @return The product name string, otherwise FALSE.
	 */
	public function getName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['name'];
	}

	/**
	 * Get the product version.
	 * @return The product version as string, otherwise FALSE.
	 */
	public function getVersion() {
		if ($this->getData() === FALSE)
			return FALSE;
		$cmd = sprintf("sudo dpkg-query -W -f='\${Version}' %s",
		  $this->getPackageName());
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		return $output[0];
	}

	/**
	 * Get the product version/release name.
	 * @return The release name as string, otherwise FALSE.
	 */
	public function getVersionName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['versionname'];
	}

	/**
	 * Get the URL to the product homepage.
	 * @return The URL, otherwise FALSE.
	 */
	public function getURL() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['url'];
	}

	/**
	 * Get the copyright text.
	 * @return The copyright text, otherwise FALSE.
	 */
	public function getCopyright() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['copyright'];
	}

	/**
	 * Get the Debian package name.
	 * @return The package name, otherwise FALSE.
	 */
	public function getPackageName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['packagename'];
	}
}
?>
