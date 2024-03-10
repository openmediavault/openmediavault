<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2024 Volker Theile
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

class OMVRpcServiceEtcd extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "Etcd";
	}

	public function initialize() {
		$this->registerMethod("getSnapshots");
		$this->registerMethod("createSnapshot");
		$this->registerMethod("deleteSnapshot");
		$this->registerMethod("restoreSnapshot");
	}

	function getSnapshots($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getlist");
		$db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.k8s");
        $sfObject = $db->get("conf.system.sharedfolder",
        	$object->get("etcdsnapshots_sharedfolderref"));
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		$snapshotDir = build_path(DIRECTORY_SEPARATOR,
			$meObject->get("dir"), $sfObject->get("reldirpath"));
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
		$result = [];
		foreach($output as $outputk => $outputv) {
			$outputv['createdAtTs'] = strpdate($outputv['createdAt'],
				"Y-m-d*H:i:sT");
			$result[] = $outputv;
		}
		return $this->applyFilter($result, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	function createSnapshot($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.k8s");
        $sfObject = $db->get("conf.system.sharedfolder",
        	$object->get("etcdsnapshots_sharedfolderref"));
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		$snapshotDir = build_path(DIRECTORY_SEPARATOR,
			$meObject->get("dir"), $sfObject->get("reldirpath"));
		$cmdArgs = [];
		$cmdArgs[] = "etcd-snapshot";
		$cmdArgs[] = "save";
		$cmdArgs[] = sprintf("--etcd-snapshot-dir %s", escapeshellarg(
			$snapshotDir));
		$cmdArgs[] = "--snapshot-compress";
		$cmd = new \OMV\System\Process("k3s", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}

	function deleteSnapshot($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.etcd.deleteSnapshot");
		$snapshots = \OMV\Rpc\Rpc::call("Etcd", "getSnapshots",
			[ "start" => 0, "limit" => -1 ], $context);
		$snapshot = array_search_ex($snapshots['data'], "name",
			$params['name']);
		if (FALSE === $snapshot) {
			throw new \OMV\Exception("The snapshot '%s' does not exist",
				$params['name']);
		}
		unlink($snapshot['location']);
	}

	function restoreSnapshot($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.etcd.restoreSnapshot");
		$snapshots = \OMV\Rpc\Rpc::call("Etcd", "getSnapshots",
			[ "start" => 0, "limit" => -1 ], $context);
		$snapshot = array_search_ex($snapshots['data'], "name",
			$params['name']);
		if (FALSE === $snapshot) {
			throw new \OMV\Exception("The snapshot '%s' does not exist",
				$params['name']);
		}
		$systemCtl = new \OMV\System\SystemCtl("k3s");
		$systemCtl->stop();
		$location = str_replace("file://", "", $snapshot['location']);
		$cmdArgs = [];
		$cmdArgs[] = "server";
		$cmdArgs[] = "--cluster-reset";
		$cmdArgs[] = sprintf("--cluster-reset-restore-path=%s",
			escapeshellarg($location));
		$cmd = new \OMV\System\Process("k3s", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$systemCtl->start();
	}
}
