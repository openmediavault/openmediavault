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

require_once("openmediavault/functions.inc");

class OMVRpcServiceNetworkUPSTools extends \OMV\Rpc\ServiceAbstract
{
    public function getName()
    {
        return "Nut";
    }

    public function initialize()
    {
        $this->registerMethod("get");
        $this->registerMethod("set");
        $this->registerMethod("getStats");
        $this->registerMethod("getVariables");
    }

    /**
     * Get the configuration settings.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return The requested configuration object.
     */
    public function get($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc("conf.service.nut");
    }

    /**
     * Set the configuration settings.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return The stored configuration object.
     */
    public function set($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.nut.set");
        // Get the existing configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.nut");
        // Update the configuration object.
        $object->setAssoc($params);
        // Put the configuration object.
        $db->set($object);
        // Return the configuration object.
        return $object->getAssoc();
    }

    /**
     * Get statistics, e.g. who's online.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return A string containing the requested statistics.
     */
    public function getStats($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.nut");
        if (false === $object->get("enable")) {
            return gettext("Service disabled");
        }
        return implode("\n", $this->getUpscOutput($object));
    }

    /**
     * Get the UPS variables as a flat "key => value" object for the
     * dashboard widgets. Both the battery gauge and the information grid
     * consume this single method; the grid maps it to rows via the
     * widget's "itemTemplates", the gauge reads the charge props.
     *
     * Variable names have their dots replaced by underscores so they can
     * be referenced in widget templates (e.g. "battery.charge" becomes
     * "battery_charge"). Numeric values are cast to numbers; duration
     * variables (e.g. "battery.runtime") stay as raw seconds and are
     * formatted in the widget template via the "duration" filter.
     *
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return An associative array of UPS variables. "battery_charge" and
     *   "battery_charge_low" are always present as numbers (the gauge
     *   needs them), defaulting to 0 when the service is disabled or the
     *   UPS does not report them.
     */
    public function getVariables($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // The gauge always needs these as numbers; default to 0 when the
        // service is disabled or the UPS does not report them.
        $result = ["battery_charge" => 0, "battery_charge_low" => 0];
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.nut");
        if (false === $object->get("enable")) {
            return $result;
        }
        $vars = $this->parseVars($this->getUpscOutput($object));
        foreach ($vars as $key => $value) {
            $result[str_replace(".", "_", $key)] = is_numeric($value) ? $value + 0 : $value;
        }
        return $result;
    }

    /**
     * Run "upsc" for the configured UPS and return its output lines.
     * @param object The "conf.service.nut" configuration object.
     * @return An array containing the lines of the "upsc" output.
     */
    private function getUpscOutput($object)
    {
        $cmdArgs = [];
        if ("netclient" === $object->get("mode")) {
            $remoteUps = $object->get("upsname")."@".$object->get("netclienthostname");
            $cmdArgs[] = escapeshellarg($remoteUps);
        } else {
            $cmdArgs[] = escapeshellarg($object->get("upsname"));
        }
        $cmd = new \OMV\System\Process("upsc", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->setRequireOutput();
        $cmd->execute($output);
        return $output;
    }

    /**
     * Parse "upsc" output into a "key => value" map. Lines without a
     * ": " separator (e.g. the "Init SSL" notice) and lines whose key
     * is not a lowercase variable name (e.g. upsd "Error:" messages)
     * are ignored.
     * @param output The "upsc" output lines.
     * @return An associative array mapping variable names to values.
     */
    private function parseVars($output)
    {
        $vars = [];
        foreach ($output as $line) {
            $parts = explode(": ", $line, 2);
            if (2 !== count($parts)) {
                continue;
            }
            $key = trim($parts[0]);
            if (!preg_match('/^[a-z][a-z0-9._]*$/', $key)) {
                continue;
            }
            $vars[$key] = trim($parts[1]);
        }
        return $vars;
    }
}
