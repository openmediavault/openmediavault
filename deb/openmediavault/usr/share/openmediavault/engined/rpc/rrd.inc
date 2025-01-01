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
namespace Engined\Rpc;

class RRD extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getGraph");
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			$cmd = new \OMV\System\Process("omv-mkrrdgraph");
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			return $output;
		});
	}

	function getGraph($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.rrd.getgraph");
		if (1 == preg_match("/\.\./", $params["kind"])) {
			throw new \OMV\Exception(
				"The field 'kind' contains forbidden two-dot symbols.");
		}
		// Build the image file path. If it does not exist, then return an
		// error image by default.
		$fileName = sprintf("%s-%s.png", $params["kind"], $params["period"]);
		$filePath = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
			"OMV_RRDGRAPH_DIR"), $fileName);
		if (!file_exists($filePath)) {
			$filePath = \OMV\Environment::get("OMV_RRDGRAPH_ERROR_IMAGE");
		}
		return [
			"filepath" => $filePath,
			"headers" => [
				"Cache-Control" => "no-store"
			]
		];
	}
}
