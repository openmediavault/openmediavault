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
namespace OMV\System\Filesystem;

class Ext extends Filesystem {
	/**
	 * See parent class definition.
	 */
	public function getDetails() {
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("dumpe2fs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	/**
	 * Grow the filesystem.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	final public function grow() {
		$this->getData();
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("resize2fs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}
}
