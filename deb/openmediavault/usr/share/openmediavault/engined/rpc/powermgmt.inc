<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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

class PowerMgmt extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "PowerMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("getScheduleList");
		$this->registerMethod("getScheduledJob");
		$this->registerMethod("setScheduledJob");
		$this->registerMethod("deleteScheduledJob");
		$this->registerMethod("executeScheduledJob");
	}

	/**
	 * Get the power management configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.system.powermngmnt");
	}

	/**
	 * Set the power management configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.powermgmt.set");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.powermngmnt");
		$object->setAssoc($params);
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get the reboot/shutdown/standby cron jobs.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 *   \em type Array of cron jobs types which are reboot, shutdown
	 *   or userdefined.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getScheduleList($params, $context) {
		return \OMV\Rpc\Rpc::call("cron", "getList", $params, $context);
	}

	/**
	 * Get a reboot/shutdown/standby cron job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getScheduledJob($params, $context) {
		return \OMV\Rpc\Rpc::call("cron", "get", $params, $context);
	}

	/**
	 * Set a reboot/shutdown/standby cron configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function setScheduledJob($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.powermgmt.setscheduledjob");
		// Set missing configuration object fields.
		$params['username'] = "root";
		$params['sendemail'] = FALSE;
		switch ($params['type']) {
		case "reboot":
			$params['command'] = "systemctl reboot";
			break;
		case "shutdown":
			$params['command'] = "systemctl poweroff";
			break;
		case "standby":
		    $pm = new \OMV\System\PowerManagement();
		    if (TRUE === $pm->isStateSupported(
		            \OMV\System\PowerManagement::STATE_SUSPENDHYBRID))
		        $params['command'] = "systemctl hybrid-sleep";
            else if (TRUE === $pm->isStateSupported(
                    \OMV\System\PowerManagement::STATE_HIBERNATE))
                $params['command'] = "systemctl hibernate";
            else if (TRUE === $pm->isStateSupported(
                    \OMV\System\PowerManagement::STATE_SUSPEND))
                $params['command'] = "systemctl suspend";
            else
                $params['command'] = "systemctl poweroff";
			break;
		}
		return \OMV\Rpc\Rpc::call("cron", "set", $params, $context);
	}

	/**
	 * Delete a reboot/shutdown/standby cron job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteScheduledJob($params, $context) {
		return \OMV\Rpc\Rpc::call("cron", "delete", $params, $context);
	}

	/**
	 * Execute a reboot/shutdown/standby cron job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	public function executeScheduledJob($params, $context) {
		return \OMV\Rpc\Rpc::call("cron", "execute", $params, $context);
	}
}
