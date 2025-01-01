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

require_once("openmediavault/functions.inc");

class OMVRpcServiceKubectl extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "Kubectl";
	}

	public function initialize() {
		$this->registerMethod("describe");
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("delete");
		$this->registerMethod("logs");
		$this->registerMethod("apply");
	}

    private function isEnabled(): bool {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.k8s");
		return $object->get("enable");
    }

	function describe($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.kubectl.describe");
		$cmdArgs = [];
		$cmdArgs[] = "describe";
		$cmdArgs[] = escapeshellarg($params['type']);
		if (!empty($params['namespace'])) {
			$cmdArgs[] = sprintf("-n %s", escapeshellarg(
				$params['namespace']));
		}
		$cmdArgs[] = escapeshellarg($params['name']);
		$cmd = new \OMV\System\Process("kubectl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	function getList($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.kubectl.getList");
		if (!$this->isEnabled()) {
		    return [];
		}
		$cmdArgs = [];
		$cmdArgs[] = "get";
		$cmdArgs[] = escapeshellarg($params['type']);
		if (!empty($params['namespace'])) {
			$cmdArgs[] = sprintf("-n %s", escapeshellarg(
				$params['namespace']));
		} else {
			$cmdArgs[] = "--all-namespaces";
		}
		if (!empty($params['format'])) {
			$cmdArgs[] = sprintf("-o %s", escapeshellarg(
				$params['format']));
		}
		$cmdArgs[] = "--no-headers";
		$cmdArgs[] = "--output json";
		$cmd = new \OMV\System\Process("kubectl", $cmdArgs);
		$cmd->setRedirect2toFile("/dev/null");
		$cmd->execute($output);
		$result = json_decode_safe(implode("", $output), TRUE);
		return $this->applyFilter($result['items'], $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	function get($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.kubectl.get");
		$result = [
			"type" => $params['type']
		];
		$cmdArgs = [];
		$cmdArgs[] = "get";
		$cmdArgs[] = escapeshellarg($params['type']);
		if (!empty($params['name'])) {
			$cmdArgs[] = escapeshellarg($params['name']);
			$result['name'] = $params['name'];
		}
		if (!empty($params['namespace'])) {
			$cmdArgs[] = sprintf("-n %s", escapeshellarg(
				$params['namespace']));
			$result['namespace'] = $params['namespace'];
		} else {
			$cmdArgs[] = "--all-namespaces";
		}
		if (!empty($params['format'])) {
			$cmdArgs[] = sprintf("-o %s", escapeshellarg(
				$params['format']));
			$result['format'] = $params['format'];
		}
		$cmd = new \OMV\System\Process("kubectl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result['manifest'] = implode("\n", $output);
		return $result;
	}

	function delete($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.kubectl.delete");
		$cmdArgs = [];
		$cmdArgs[] = "delete";
		$cmdArgs[] = escapeshellarg($params['type']);
		if (!empty($params['name'])) {
			$cmdArgs[] = escapeshellarg($params['name']);
		}
		if (!empty($params['namespace'])) {
			$cmdArgs[] = sprintf("-n %s", escapeshellarg(
				$params['namespace']));
		} else {
			$cmdArgs[] = "--all-namespaces";
		}
		if (!empty($params['format'])) {
			$cmdArgs[] = sprintf("-o %s", escapeshellarg(
				$params['format']));
		}
		$cmd = new \OMV\System\Process("kubectl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	public function logs($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.kubectl.logs");
		$cmdArgs = [];
		$cmdArgs[] = "logs";
		if (!empty($params['namespace'])) {
			$cmdArgs[] = sprintf("-n %s", escapeshellarg(
				$params['namespace']));
		}
		$cmdArgs[] = escapeshellarg($params['name']);
		$cmd = new \OMV\System\Process("kubectl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	public function apply($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.kubectl.apply");
		$tmpFile = new \OMV\System\TmpFile();
		$tmpFile->write($params['manifest']);
		$cmdArgs = [];
		$cmdArgs[] = "apply";
		if (!empty($params['namespace'])) {
			$cmdArgs[] = sprintf("-n %s", escapeshellarg(
				$params['namespace']));
		}
		$cmdArgs[] = sprintf("-f %s", escapeshellarg(
			$tmpFile->getFilename()));
		$cmd = new \OMV\System\Process("kubectl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}
}
