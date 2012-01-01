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

class OMVProduct {
	private $info = array();

	/**
	 * Get the product informations, e.g. name, version and URL.
	 * @return An array containing the product informations.
	 * array(
	 *   name => xxx,
	 *   version => xxx,
	 *   versionname => xxx,
	 *   url => xxx,
	 *   copyright => xxx
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

	public function getName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['name'];
	}

	public function getVersion() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['version'];
	}

	public function getVersionName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['versionname'];
	}

	public function getURL() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['url'];
	}

	public function getCopyright() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->info['copyright'];
	}
}
?>
