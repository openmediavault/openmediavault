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

class OMVRpcServicePodman extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "Podman";
	}

	public function initialize() {
		$this->registerMethod("imageList");
		$this->registerMethod("imageListBg");
		$this->registerMethod("imageInspect");
		$this->registerMethod("imagePullBg");
		$this->registerMethod("imageDelete");
		$this->registerMethod("imageTags");
		$this->registerMethod("containerList");
		$this->registerMethod("containerListBg");
		$this->registerMethod("containerInspect");
		$this->registerMethod("containerRestart");
		$this->registerMethod("containerLogs");
		$this->registerMethod("containerStats");
		$this->registerMethod("volumeList");
		$this->registerMethod("volumeListBg");
		$this->registerMethod("volumeInspect");
		$this->registerMethod("podList");
		$this->registerMethod("podListBg");
		$this->registerMethod("podInspect");
		$this->registerMethod("podRestart");
		$this->registerMethod("podStats");
		$this->registerMethod("podLogs");
	}

	function imageList($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the list of available images.
		$cmdArgs = [];
		$cmdArgs[] = "image";
		$cmdArgs[] = "list";
		$cmdArgs[] = "--noheading";
		$cmdArgs[] = "--no-trunc";
		$cmdArgs[] = "--format 'table {{.Containers}} {{.Repository}} {{.Tag}} {{.Id}} {{.VirtualSize}} {{.Digest}} {{.CreatedAt}}'";
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = [];
		foreach ($output as $outputk => $outputv) {
			$regex = "/^(\d+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(.+)$/";
			if (1 !== preg_match($regex, $outputv, $matches)) {
				continue;
			}
			$result[] = [
				"Containers" => intval($matches[1]),
				"Repository" => $matches[2],
				"Tag" => $matches[3],
				"Id" => $matches[4],
				"VirtualSize" => intval($matches[5]),
				"CreatedAt" => $matches[7],
				"Digest" => $matches[6],
				"CreatedTs" => strpdate($matches[7],"Y-m-d H:i:s O T"),
				"_used" => intval($matches[1]) > 0
			];
		}
		// Filter result.
		return $this->applyFilter($result, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	public function imageListBg($params, $context) {
		return $this->callMethodBg("imageList", $params, $context);
	}

	public function imageInspect($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "image";
		$cmdArgs[] = "inspect";
		$cmdArgs[] = "--format json";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	public function imagePullBg($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.source");
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  		use ($params) {
			$cmdArgs = [];
			$cmdArgs[] = "image";
			$cmdArgs[] = "pull";
			$cmdArgs[] = escapeshellarg($params['source']);
			$cmd = new \OMV\System\Process("podman", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
		});
	}

	public function imageDelete($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "image";
		$cmdArgs[] = "rm";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}

	public function imageTags($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.name");
		$cmdArgs = [];
		$cmdArgs[] = "image";
		$cmdArgs[] = "search";
		$cmdArgs[] = "--list-tags";
		$cmdArgs[] = "--format json";
		$cmdArgs[] = escapeshellarg($params['name']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = json_decode_safe(implode("", $output), TRUE)[0];
		return array_value($result, "Tags", []);
	}

	function containerList($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the list of available containers.
		$cmdArgs = [];
		$cmdArgs[] = "container";
		$cmdArgs[] = "ls";
		$cmdArgs[] = "--format json";
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = json_decode_safe(implode("", $output), TRUE);
		// Filter result.
		return $this->applyFilter($result, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	public function containerListBg($params, $context) {
		return $this->callMethodBg("containerList", $params, $context);
	}

	public function containerRestart($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "container";
		$cmdArgs[] = "restart";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}

	public function containerInspect($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "container";
		$cmdArgs[] = "inspect";
		$cmdArgs[] = "--format json";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	public function containerLogs($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "container";
		$cmdArgs[] = "logs";
		$cmdArgs[] = "--names";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	public function containerStats($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.containerStats");
		$cmdArgs = [];
		$cmdArgs[] = "container";
		$cmdArgs[] = "stats";
		$cmdArgs[] = "--no-stream";
		$cmdArgs[] = "--format json";
		if (array_key_exists("id", $params)) {
			$cmdArgs[] = escapeshellarg($params['id']);
		}
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return json_decode_safe(implode("", $output), TRUE);
	}

	function volumeList($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the list of available containers.
		$cmdArgs = [];
		$cmdArgs[] = "volume";
		$cmdArgs[] = "ls";
		$cmdArgs[] = "--format json";
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = json_decode_safe(implode("", $output), TRUE);
		// Filter result.
		return $this->applyFilter($result, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	public function volumeListBg($params, $context) {
		return $this->callMethodBg("volumeList", $params, $context);
	}

	public function volumeInspect($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.volume");
		$cmdArgs = [];
		$cmdArgs[] = "volume";
		$cmdArgs[] = "inspect";
		$cmdArgs[] = "--format json";
		$cmdArgs[] = escapeshellarg($params['volume']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	function podList($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the list of available containers.
		$cmdArgs = [];
		$cmdArgs[] = "pod";
		$cmdArgs[] = "ps";
		$cmdArgs[] = "--format json";
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = json_decode_safe(implode("", $output), TRUE);
		// Filter result.
		return $this->applyFilter($result, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	public function podListBg($params, $context) {
		return $this->callMethodBg("podList", $params, $context);
	}

	public function podInspect($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "pod";
		$cmdArgs[] = "inspect";
		$cmdArgs[] = "--format json";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	public function podRestart($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "pod";
		$cmdArgs[] = "restart";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}

	public function podStats($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "pod";
		$cmdArgs[] = "stats";
		$cmdArgs[] = "--no-stream";
		$cmdArgs[] = "--format json";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return json_decode_safe(implode("", $output), TRUE);
	}

	public function podLogs($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.podman.id");
		$cmdArgs = [];
		$cmdArgs[] = "pod";
		$cmdArgs[] = "logs";
		$cmdArgs[] = "--names";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("podman", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}
}
