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
namespace OMV\Engine\Module;

/**
 * A module class that implements the default methods defined by the
 * \OMV\Engine\Module\IService interface.
 * @ingroup api
 */
abstract class ServiceAbstract extends ModuleAbstract implements IService {
	/**
	 * Defines the modules that if present should build their configuration
	 * before the service provided by this module.
	 * @return An array of modules.
	 */
	public function shouldApply() { return []; }

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return []; }

	/**
	 * Defines the modules that if present should start after the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	function startBefore() { return []; }

	/**
	 * Defines the modules that if present should be stopped after the
	 * service provided by this module.
	 * @return An array of modules.
	 */
	function shouldStop() { return []; }

	/**
	 * Defines the modules that if present should stop before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	function stopAfter() { return []; }

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {}

	/**
	 * Start the managed service.
	 */
	public function startService() {}
}
