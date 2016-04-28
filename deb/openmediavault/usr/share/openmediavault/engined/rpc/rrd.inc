<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
class OMVRpcServiceRrd extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Rrd";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("generate");
	}

	/**
	 * Generate the RRD graphs.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\ExecException
	 */
	function generate($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if ($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$cmd = new \OMV\System\Process("omv-mkgraph");
			$cmd->setRedirect2to1(TRUE);
			$cmd->execute($output);
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(\Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}
