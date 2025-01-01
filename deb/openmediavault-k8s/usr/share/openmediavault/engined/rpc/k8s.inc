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
require_once("openmediavault/vendor/autoload.php");

class OMVRpcServiceK8s extends \OMV\Rpc\ServiceAbstract {
	private $recipesRepoDir = "/var/lib/openmediavault/k8s-recipes/";

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
		$this->registerMethod("refreshRecipes");
		$this->registerMethod("listRecipes");
		$this->registerMethod("getRecipe");
		$this->registerMethod("applyRecipe");
	}

	private function isK3sInstalled(): bool {
		return is_executable("/usr/local/bin/k3s");
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
		$response = \OMV\Rpc\Rpc::call("Config", "get", [
			"id" => "conf.service.k8s"
		], $context);
		$response['installed'] = $this->isK3sInstalled();
		return $response;
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
		$response = \OMV\Rpc\Rpc::call("Kubectl", "get", [
			"type" => "secret",
			"name" => "admin-user",
			"namespace" => "kubernetes-dashboard",
			"format" => "json"
		], $context);
		$secret = json_decode_safe($response['manifest'], TRUE);
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
			"unlink" => FALSE,
			"headers" => [
				"Content-Type" => "text/yaml"
			],
		];
	}

	function getManifestIngredients($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$db = \OMV\Config\Database::getInstance();
		$sfObjects = $db->get("conf.system.sharedfolder");
		$sfs = [];
		foreach ($sfObjects as $sfObjectk => $sfObjectv) {
			$meObject = $db->get("conf.system.filesystem.mountpoint",
				  $sfObjectv->get("mntentref"));
			$sfs[] = [
				"name" => $sfObjectv->get("name"),
				"absdirpath" => build_path(DIRECTORY_SEPARATOR,
					$meObject->get("dir"), $sfObjectv->get("reldirpath")),
				"comment" => $sfObjectv->get("comment")
			];
		}
		$users = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateAllUsers",
			null, $context);
		$groups = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateAllGroups",
			null, $context);
		return [
			"sharedfolders" => $sfs,
			"users" => $users,
			"groups" => $groups
		];
	}

	function refreshRecipes($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$cmdArgs = [];
		$cmdArgs[] = sprintf("-C %s", escapeshellarg($this->recipesRepoDir));
		$cmdArgs[] = "pull";
		$cmdArgs[] = "origin";
		$cmdArgs[] = "main";
		$cmd = new \OMV\System\Process("git", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}

	function listRecipes($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.common.getList");
		$result = [];
		if (is_dir($this->recipesRepoDir)) {
			$recipesDir = build_path(DIRECTORY_SEPARATOR, $this->recipesRepoDir,
				"recipes");
			foreach (new \DirectoryIterator($recipesDir) as $file) {
				if ($file->isDot()) {
					continue;
				}
				if (!$file->isDir()) {
					continue;
				}
				$path = build_path(DIRECTORY_SEPARATOR, $this->recipesRepoDir,
					"recipes", $file->getBasename(), "metadata.yaml");
				if (!file_exists($path)) {
					continue;
				}
				$result[] = array_merge([ "id" => $file->getBasename() ],
					yaml_parse_file($path));
			}
		}
		return $this->applyFilter($result, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	function getRecipe($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.k8s.getRecipe");
		$path = build_path(DIRECTORY_SEPARATOR, $this->recipesRepoDir,
			"recipes", $params['id'], "recipe.yaml");
		if (!file_exists($path)) {
			throw new \OMV\Exception(
				"The requested recipe does not exist.");
		}
		return [
			"id" => $params['id'],
			"namespace" => "",
			"manifest" => file_get_contents($path)
		];
	}

	function applyRecipe($params, $context) {
		// Remove commented lines.
		$params['manifest'] = preg_replace("/^\s*#.*$/m", "",
			$params['manifest']);
		// Replace placeholders in the recipe.
		$db = \OMV\Config\Database::getInstance();
		$functions = [
			"hostname" => function() use($db) {
				$obj = $db->get("conf.system.network.dns");
				return $obj->get("hostname");
			},
			"domain" => function() use($db) {
				$obj = $db->get("conf.system.network.dns");
				return $obj->get("domainname");
			},
			"fqdn" => function() use($db) {
				$obj = $db->get("conf.system.network.dns");
				if (!$obj->isEmpty("domainname")) {
					$result = sprintf("%s.%s", $obj->get("hostname"),
						$obj->get("domainname"));
				} else {
					$result = $obj->get("hostname");
				}
				return $result;
			},
			"ipaddr" => function($deviceName = NULL) {
				if (empty($deviceName)) {
					$cmdArgs = [];
					$cmdArgs[] = "-4";
					$cmdArgs[] = "-json";
					$cmdArgs[] = "route";
					$cmdArgs[] = "show";
					$cmdArgs[] = "default";
					$cmd = new \OMV\System\Process("ip", $cmdArgs);
					$cmd->setRedirect2to1();
					$cmd->execute($output);
					$output = json_decode_safe(implode("", $output), TRUE);
					if (empty($output)) {
						throw new \OMV\Exception("No default routes found.");
					}
					return $output[0]['prefsrc'];
				} else {
					$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
					$iface = $mngr->assertGetImpl($deviceName);
					if (FALSE === ($ipAddr = $iface->getIP())) {
						throw new \OMV\Exception(
							"The network interface does not have an assigned ".
							"IPv4 address (name=%s).", $deviceName);
					}
					return $ipAddr;
				}
			},
			"ipaddr6" => function($deviceName = NULL) {
				if (empty($deviceName)) {
					$cmdArgs = [];
					$cmdArgs[] = "-6";
					$cmdArgs[] = "-json";
					$cmdArgs[] = "route";
					$cmdArgs[] = "show";
					$cmdArgs[] = "default";
					$cmd = new \OMV\System\Process("ip", $cmdArgs);
					$cmd->setRedirect2to1();
					$cmd->execute($output);
					$output = json_decode_safe(implode("", $output), TRUE);
					if (empty($output)) {
						throw new \OMV\Exception("No default routes found.");
					}
					return $output[0]['prefsrc'];
				} else {
					$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
					$iface = $mngr->assertGetImpl($deviceName);
					if (FALSE === ($ipAddr6 = $iface->getIP6())) {
						throw new \OMV\Exception(
							"The network interface does not have an assigned ".
							"IPv6 address (name=%s).", $deviceName);
					}
					return $ipAddr6;
				}
			},
			"tz" => function() use($db) {
				$obj = $db->get("conf.system.time");
				return $obj->get("timezone");
			},
			"uid" => function($name) use($context) {
				$result = \OMV\Rpc\Rpc::call("UserMgmt", "getUser", [
					"name" => $name
				], $context);
				return $result['uid'];
			},
			"gid" => function($name) use($context) {
				$result = \OMV\Rpc\Rpc::call("UserMgmt", "getGroup", [
					"name" => $name
				], $context);
				return $result['gid'];
			},
			"sharedfolder_path" => function($name) use($db) {
				$sfObject = $db->getByFilter("conf.system.sharedfolder", [
					"operator" => "stringEquals",
					"arg0" => "name",
					"arg1" => $name
				], 1);
				$meObject = $db->get("conf.system.filesystem.mountpoint",
					$sfObject->get("mntentref"));
				return build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
					$sfObject->get("reldirpath"));
			},
			"conf_get" => function($id, $uuid = NULL) use($db) {
				return $db->getAssoc($id, $uuid);
			}
		];
		$filters = [
			"get" => function($value, $key, $default = NULL) {
				$dict = new \OMV\Dictionary($value);
				return $dict->get($key, $default);
			}
		];
		$loader = new \Twig\Loader\ArrayLoader([
			'manifest' => $params['manifest']
		]);
		$twig = new \Twig\Environment($loader);
		foreach ($functions as $name => $callback) {
			$function = new \Twig\TwigFunction($name, $callback);
			$twig->addFunction($function);
		}
		foreach ($filters as $name => $callback) {
			$filter = new \Twig\TwigFilter($name, $callback);
			$twig->addFilter($filter);
		}
		$params['manifest'] = $twig->render("manifest");
		return \OMV\Rpc\Rpc::call("Kubectl", "apply", $params, $context);
	}
}
