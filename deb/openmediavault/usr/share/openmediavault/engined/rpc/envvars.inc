<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
 * along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
 */
namespace Engined\Rpc;

class EnvVars extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "EnvVars";
	}

	public function initialize() {
		$this->registerMethod("enumerate");
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("apply");
		$this->registerMethod("applyBg");
	}

	/**
	 * Get all available environment variable names.
	 * @param array $params Optional method parameters.
	 * @param array $context The context of the caller.
	 * @return array An array containing all available environment variable names.
	 */
	public function enumerate($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$cmdArgs = [];
		$cmdArgs[] = "list";
		$cmdArgs[] = "--json";
		$cmd = new \OMV\System\Process("omv-env", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return json_decode_safe(implode("", $output));
	}

	/**
	 * Get the configured environment variables.
	 * @param array $params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param array $context The context of the caller.
	 * @return array An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array.
	 */
	public function getList($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getlist");
		$cmdArgs = [];
		$cmdArgs[] = "get";
		$cmdArgs[] = "--json";
		$cmd = new \OMV\System\Process("omv-env", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$output = json_decode_safe(implode("", $output));
		$result = [];
		foreach ($output as $name => $value) {
			$result[] = [
				"name" => $name,
				"value" => $value
			];
		}
		return $this->applyFilter($result, $params['start'], $params['limit'],
			$params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get an environment variable.
	 * @param array $params An array containing the following fields:
	 *   \em name The name of the environment variable.
	 * @param array $context The context of the caller.
	 * @return array The name and value of the requested environment variable.
	 * @throw \OMV\Exception
	 */
	public function get($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.envvars.get");
		$cmdArgs = [];
		$cmdArgs[] = "get";
		$cmdArgs[] = "--value-only";
		$cmdArgs[] = escapeshellarg($params['name']);
		$cmd = new \OMV\System\Process("omv-env", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return [
			"name" => $params['name'],
			"value" => implode("\n", $output)
		];
	}

	/**
	 * Set an environment variable.
	 * @param array $params An array containing the following fields:
	 *   \em name The name of the environment variable.
	 *   \em value The value of the environment variable.
	 * @param array $context The context of the caller.
	 * @return array The stored environment variable.
	 */
	public function set($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.envvars.set");
		$cmdArgs = [];
		$cmdArgs[] = "set";
		$cmdArgs[] = "--";
		$cmdArgs[] = escapeshellarg($params['name']);
		$cmdArgs[] = escapeshellarg($params['value']);
		$cmd = new \OMV\System\Process("omv-env", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return array_pick_keys($params, ["name", "value"]);
	}

	/**
	 * Delete an environment variable.
	 * @param array $params An array containing the following fields:
	 *   \em name The name of the environment variable.
	 * @param array $context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function delete($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.envvars.delete");
		$cmdArgs = [];
		$cmdArgs[] = "unset";
		$cmdArgs[] = escapeshellarg($params['name']);
		$cmd = new \OMV\System\Process("omv-env", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}

	/**
	 * Apply the configured environment variables.
	 * @param array $params Optional method parameters.
	 * @param array $context The context of the caller.
	 * @return void
	 */
	public function apply($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$cmdArgs = [];
		$cmdArgs[] = "stage";
		$cmdArgs[] = "run";
		$cmdArgs[] = "prepare";
		$cmd = new \OMV\System\Process("omv-salt", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$cmdArgs = [];
		$cmdArgs[] = "stage";
		$cmdArgs[] = "run";
		$cmdArgs[] = "deploy";
		$cmd = new \OMV\System\Process("omv-salt", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$cmdArgs = [];
		$cmdArgs[] = "restart";
		$cmdArgs[] = "omv-engined";
		$cmd = new \OMV\System\Process("monit", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}

	/**
	 * Apply the configured environment variables in a background process.
	 * @param array $params The method parameters. \see apply.
	 * @param array $context The context of the caller.
	 * @return string The name of the background process status file.
	 */
	public function applyBg($params, $context) {
		return $this->callMethodBg("apply", $params, $context);
	}
}
