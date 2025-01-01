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

class OMVRpcServiceK3s extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "K3s";
	}

	public function initialize() {
		$this->registerMethod("getSnapshotList");
		$this->registerMethod("createSnapshot");
		$this->registerMethod("deleteSnapshot");
		$this->registerMethod("restoreSnapshot");
	}

	private function isK3sInstalled(): bool {
		return is_executable("/usr/local/bin/k3s");
	}

    private function getConfig() {
        $db = \OMV\Config\Database::getInstance();
        return $db->get("conf.service.k8s");
    }

	private function isEtcd(): bool {
		$object = $this->getConfig();
		return "etcd" == $object->get("datastore");
	}

    private function getSnapshotDir(): string {
		$db = \OMV\Config\Database::getInstance();
        $object = $this->getConfig();
        if ($object->isEmpty("snapshots_sharedfolderref")) {
			throw new \OMV\Config\Exception(
				"The shared folder for snapshots is not configured");
		}
        $sfObject = $db->get("conf.system.sharedfolder",
        	$object->get("snapshots_sharedfolderref"));
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		return build_path(DIRECTORY_SEPARATOR,
			$meObject->get("dir"), $sfObject->get("reldirpath"));
    }

	function getSnapshotList($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getlist");
		$result = [];
		$snapshotDir = $this->getSnapshotDir();
		if ($this->isEtcd()) {
			$cmdArgs = [];
			$cmdArgs[] = "etcd-snapshot";
			$cmdArgs[] = "ls";
			$cmdArgs[] = sprintf("--etcd-snapshot-dir %s", escapeshellarg(
				$snapshotDir));
			$cmdArgs[] = "--output json";
			$cmd = new \OMV\System\Process("k3s", $cmdArgs);
			$cmd->setRedirect2toFile("/dev/null");
			$cmd->execute($output);
			$output = json_decode_safe(implode("", $output), TRUE);
			$output = array_value($output, "items", []);
			foreach($output as $outputk => $outputv) {
				$outputv['status']['compressed'] = str_ends_with(
					$outputv['spec']['snapshotName'], ".zip");
				if (is_null($outputv['metadata']['creationTimestamp'])) {
					$outputv['metadata']['creationTimestamp'] = strpdate(
						$outputv['status']['creationTime'],
						"Y-m-d*H:i:sT");
				}
				$result[] = $outputv;
			}
		} else {
			$pattern = build_path(DIRECTORY_SEPARATOR, $snapshotDir, "*");
			foreach (glob($pattern) as $path) {
				if (!is_file($path)) {
					continue;
				}
				$name = basename($path);
				$ts = filemtime($path);
				$result[] = [
					"kind" => "SQLiteSnapshotFile",
					"metadata" => [
						"name" => $name,
						"creationTimestamp" => $ts
					],
					"spec" => [
						"snapshotName" => $name,
						"nodeName" => \OMV\System\Net\Dns::getHostname(),
						"location" => "file://" . $path
					],
					"status" => [
						"size" => filesize($path),
						"creationTime" => date("Y-m-dTH:i:sZ", $ts),
						"readyToUse" => TRUE,
						"compressed" => str_ends_with($path, ".tar.xz")
					]
				];
			}
		}
		return $this->applyFilter($result, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Create a snapshot of the K3s cluster.
	 * References:
	 * - https://docs.k3s.io/cli/etcd-snapshot
	 * - https://docs.k3s.io/datastore/backup-restore#backup-and-restore-with-sqlite
	 */
	function createSnapshot($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		if (!$this->isK3sInstalled()) {
			throw new \OMV\Exception("K3s is not installed.");
		}
		$snapshotDir = $this->getSnapshotDir();
		if ($this->isEtcd()) {
			$cmdArgs = [];
			$cmdArgs[] = "etcd-snapshot";
			$cmdArgs[] = "save";
			$cmdArgs[] = sprintf("--etcd-snapshot-dir %s", escapeshellarg(
				$snapshotDir));
			$cmdArgs[] = "--snapshot-compress";
			$cmd = new \OMV\System\Process("k3s", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		} else {
			$fileName = build_path(DIRECTORY_SEPARATOR, $snapshotDir,
				sprintf("on-demand-%s-%d.tar.xz",
				\OMV\System\Net\Dns::getHostname(), time()));
			$cmdArgs = [];
			$cmdArgs[] = "--directory=/var/lib/rancher/k3s/server/";
			$cmdArgs[] = "--create";
			$cmdArgs[] = "--recursion";
			$cmdArgs[] = "--xz";
			$cmdArgs[] = sprintf("--file=%s", escapeshellarg($fileName));
			$cmdArgs[] = "token";
			$cmdArgs[] = "db/";
			$cmd = new \OMV\System\Process("tar", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		}
	}

	function deleteSnapshot($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.k3s.deleteSnapshot");
		$response = \OMV\Rpc\Rpc::call("K3s", "getSnapshotList",
			[ "start" => 0, "limit" => -1 ], $context);
		$snapshots = array_map("array_flatten", $response['data']);
		$snapshot = array_search_ex($snapshots, "spec.snapshotName",
			$params['name']);
		if (FALSE === $snapshot) {
			throw new \OMV\Exception("The snapshot '%s' does not exist.",
				$params['name']);
		}
		$snapshotDir = $this->getSnapshotDir();
		if ($this->isEtcd()) {
			$cmdArgs = [];
			$cmdArgs[] = "etcd-snapshot";
			$cmdArgs[] = "delete";
			$cmdArgs[] = sprintf("--etcd-snapshot-dir %s", escapeshellarg(
				$snapshotDir));
			$cmdArgs[] = escapeshellarg($params['name']);
			$cmd = new \OMV\System\Process("k3s", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		} else {
			$fileName = build_path(DIRECTORY_SEPARATOR, $snapshotDir,
				$snapshot['spec.snapshotName']);
			unlink($fileName);
		}
	}

	/**
	 * Restore a snapshot of the K3s cluster.
	 * References:
	 * - https://docs.k3s.io/cli/etcd-snapshot
	 * - https://docs.k3s.io/datastore/backup-restore#backup-and-restore-with-sqlite
	 */
	function restoreSnapshot($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.K3s.restoreSnapshot");
		$response = \OMV\Rpc\Rpc::call("K3s", "getSnapshotList",
			[ "start" => 0, "limit" => -1 ], $context);
		$snapshots = array_map("array_flatten", $response['data']);
		$snapshot = array_search_ex($snapshots, "spec.snapshotName",
			$params['name']);
		if (FALSE === $snapshot) {
			throw new \OMV\Exception("The snapshot '%s' does not exist.",
				$params['name']);
		}
		$snapshotDir = $this->getSnapshotDir();
		$fileName = build_path(DIRECTORY_SEPARATOR, $snapshotDir,
			$snapshot['spec.snapshotName']);
		$systemCtl = new \OMV\System\SystemCtl("k3s");
		$systemCtl->stop();
		if ($this->isEtcd()) {
			$cmdArgs = [];
			$cmdArgs[] = "server";
			$cmdArgs[] = "--cluster-reset";
			$cmdArgs[] = sprintf("--cluster-reset-restore-path=%s",
				escapeshellarg($fileName));
			$cmd = new \OMV\System\Process("k3s", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		} else {
			rmdir("/var/lib/rancher/k3s/server/db/");
			$cmdArgs = [];
			$cmdArgs[] = "--directory=/var/lib/rancher/k3s/server/";
			$cmdArgs[] = "--extract";
			$cmdArgs[] = sprintf("--file=%s", escapeshellarg($fileName));
			$cmd = new \OMV\System\Process("tar", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		}
		$systemCtl->start();
	}
}
