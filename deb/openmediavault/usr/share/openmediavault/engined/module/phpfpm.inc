<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
class OMVModulePhp5fpm extends \OMV\Engine\Module\ServiceAbstract {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "phpfpm";
	}

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		// Build the configuration.
		$cmd = new \OMV\System\Process("omv-mkconf", "php-fpm");
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Test the new configuration.
		$cmd = new \OMV\System\Process("php-fpm7.0", "-t");
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$systemCtl = new \OMV\System\SystemCtl("php7.0-fpm");
		$systemCtl->reload();
	}
}
