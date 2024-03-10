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
class OMVRpcServiceK8s extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "K8s";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("isConfigApplicable");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("getToken");
		$this->registerMethod("getKubeConfig");
		$this->registerMethod("getManifestIngredients");
	}

    /**
     * Checks whether the service is activated and the corresponding
     * module is not marked as dirty.
     */
	function isConfigApplicable($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
	    $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.k8s");
		if (FALSE === $object->get("enable")) {
			throw new \OMV\Exception("The service is disabled.");
		}
		if ($this->isModuleDirty("k3s")) {
			throw new \OMV\Config\ConfigDirtyException();
		}
	}

	/**
	 * Get the configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function get($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "get", [
			"id" => "conf.service.k8s"
		], $context);
	}

	/**
	 * Set the configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function set($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "set", [
			"id" => "conf.service.k8s",
			"data" => $params
		], $context);
	}

	function getToken($params, $context) {
		\OMV\Rpc\Rpc::call("K8s", "isConfigApplicable", NULL, $context);
		$secret = \OMV\Rpc\Rpc::call("Kubectl", "get", [
			"type" => "secret",
			"name" => "admin-user",
			"namespace" => "kubernetes-dashboard",
			"format" => "json"
		], $context);
		$secret = json_decode_safe($secret, TRUE);
		return [
			"token" => base64_decode($secret['data']['token'])
		];
	}

	function getKubeConfig($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$filePath = "/etc/rancher/k3s/k3s.yaml";
		if (!is_readable($filePath)) {
			throw new \OMV\Exception("Kubeconfig not readable.");
		}
		// Return values required by generic download RPC implementation.
		return [
			"filename" => "kubeconfig.yaml",
			"filepath" => $filePath,
			"contenttype" => "text/yaml",
			"unlink" => FALSE
		];
	}

	function getManifestIngredients($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$db = \OMV\Config\Database::getInstance();
		$sfObjects = $db->get("conf.system.sharedfolder");
		$pvs = [];
		foreach ($sfObjects as $sfObjectk => $sfObjectv) {
			$name = sprintf("shared-folder-%s", mb_strtolower(str_replace(
				"_", "-", $sfObjectv->get("name"))));
			$pvs[] = [
				"name" => $name,
				"comment" => $sfObjectv->get("comment")
			];
		}
		$users = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateAllUsers",
			null, $context);
		$groups = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateAllGroups",
			null, $context);
		return [
			"pvs" => $pvs,
			"users" => $users,
			"groups" => $groups
		];
	}
}
