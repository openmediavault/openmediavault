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
 * A module class that implements the default methods defined by the
 * \OMV\Engine\Module\IService interface.
 * @ingroup api
 */
abstract class ServiceAbstract extends ModuleAbstract implements IService {
	/**
	 * Ensures that this module is deployed after the listed modules.
	 * Nevertheless, the module can still be deployed if the listed
	 * dependencies are missing.
	 * @return array A list of module names. Returns an empty array if no
	 *   dependencies exist.
	 */
	public function deployAfter(): array { return []; }

	/**
	 * Ensures that this module is deployed before the listed modules.
	 * Nevertheless, the module can still be deployed if the listed
	 * dependencies are missing.
	 * @return array A list of module names. Returns an empty array if no
	 *   dependencies exist.
	 */
	public function deployBefore(): array { return []; }

	/**
	 * This method is called before the service is deployed.
	 */
	public function preDeploy() {}

	/**
	 * Deploy the service.
	 */
	public function deploy() {
		$name = strtolower($this->getName());
		$cmd = new \OMV\System\Process("omv-salt", [
			"deploy", "run", "--no-color", $name
		]);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * This method is called after the module has been deployed.
	 */
	public function postDeploy() {}
}
