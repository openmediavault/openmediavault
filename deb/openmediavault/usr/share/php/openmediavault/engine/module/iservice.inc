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
namespace OMV\Engine\Module;

/**
 * Interface to implement a service managed by the module.
 * @ingroup api
 */
interface IService {
	/**
	 * Ensures that this module is deployed after the listed modules.
	 * Nevertheless, the module can still be deployed if the listed
	 * dependencies are missing.
	 * @return array A list of module names. Returns an empty array if no
	 *   dependencies exist.
	 */
	public function deployAfter(): array;

	/**
	 * Ensures that this module is deployed before the listed modules.
	 * Nevertheless, the module can still be deployed if the listed
	 * dependencies are missing.
	 * @return array A list of module names. Returns an empty array if no
	 *   dependencies exist.
	 */
	public function deployBefore(): array;

	/**
	 * This method is called before the module is deployed.
	 */
	public function preDeploy();

	/**
	 * Deploy the configuration.
	 */
	public function deploy();

	/**
	 * This method is called after the module has been deployed.
	 */
	public function postDeploy();
}
