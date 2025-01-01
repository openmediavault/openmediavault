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

class OMVRpcServiceCrictl extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "Crictl";
	}

	public function initialize() {
		$this->registerMethod("images");
		$this->registerMethod("rmi");
	}

    private function isEnabled(): bool {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.k8s");
		return $object->get("enable");
    }

	function images($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getlist");
		if (!$this->isEnabled()) {
		    return [];
		}
		$cmdArgs = [];
		$cmdArgs[] = "images";
		$cmdArgs[] = "--output json";
		$cmd = new \OMV\System\Process("crictl", $cmdArgs);
		$cmd->setRedirect2toFile("/dev/null");
		$cmd->execute($output);
		$result = json_decode_safe(implode("", $output), TRUE);
		return $this->applyFilter($result['images'], $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	function rmi($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.crictl.id");
		if (!$this->isEnabled()) {
		    return [];
		}
		$cmdArgs = [];
		$cmdArgs[] = "rmi";
		$cmdArgs[] = escapeshellarg($params['id']);
		$cmd = new \OMV\System\Process("crictl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}
}
