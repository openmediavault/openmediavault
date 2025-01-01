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
namespace OMV\System\Filesystem\Backend;

/**
 * References:
 * - https://en.wikipedia.org/wiki/Reiser4
 * - https://archive.kernel.org/oldwiki/reiser4.wiki.kernel.org/
 * - https://wiki.archlinux.org/title/Reiser4/
 */
class Reiser4 extends BackendAbstract {
	public function __construct() {
		$this->type = "reiser4";
		$this->properties = self::PROP_MNTENT;
		$this->mntOptions = explode(",", \OMV\Environment::get(
			"OMV_FSTAB_MNTOPS_REISER4"));
	}

	/**
	 * See parent class definition.
	 */
	public function getFstabMntOptions(
			\OMV\System\Storage\StorageDevice $sd = null) {
		$options = parent::getFstabMntOptions($sd);
		if (!is_null($sd)) {
			if (FALSE === $sd->isRotational()) {
				$options[] = "txmod=wa";
			}
		}
		return $options;
	}
}
