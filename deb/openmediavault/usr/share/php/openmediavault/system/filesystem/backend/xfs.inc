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

class Xfs extends BackendAbstract {
	public function __construct() {
		$this->type = "xfs";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL |
		  self::PROP_DISCARD | self::PROP_QUOTA | self::PROP_RESIZE;
		$this->mkfsOptions = \OMV\Environment::get("OMV_MKFS_OPTIONS_XFS");
		$this->mntOptions = explode(",", \OMV\Environment::get(
		  "OMV_FSTAB_MNTOPS_XFS"));
	}

	/**
	 * See parent class definition.
	 */
	public function getFstabMntOptions(
	  \OMV\System\Storage\StorageDevice $sd = null) {
		$options = parent::getFstabMntOptions($sd);
		if (TRUE === is_64bits())
			$options[] = "inode64";
		return $options;
	}

	/**
	 * See parent class definition.
	 */
	function getImpl($args) {
		$object = new \OMV\System\Filesystem\Xfs($args);
		$object->setBackend($this);
		return $object;
	}
}
