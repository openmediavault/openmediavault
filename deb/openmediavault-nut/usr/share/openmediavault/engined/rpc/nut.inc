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
        $this->registerMethod("getBatteryStats");
        $this->registerMethod("getInformation");
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
     * Get the parsed battery statistics for the dashboard widget.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return An object with the "battery.*" variables (dots replaced
     *   by underscores, numeric values cast). The "battery_charge" and
     *   "battery_charge_low" fields the gauge needs are always present,
     *   defaulting to 0 when the service is disabled or the UPS does
     *   not report them.
     */
    public function getBatteryStats($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // The gauge always needs these as numbers; default to 0 when the
        // service is disabled or the UPS does not report them.
        $stats = ["battery_charge" => 0, "battery_charge_low" => 0];
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.nut");
        if (false === $object->get("enable")) {
            return $stats;
        }
        $vars = $this->parseVars($this->getUpscOutput($object));
        foreach ($vars as $key => $value) {
            if (!str_starts_with($key, "battery.")) {
                continue;
            }
            $stats[str_replace(".", "_", $key)] = is_numeric($value) ? $value + 0 : $value;
        }
        return $stats;
    }

    /**
     * Get curated device information for the dashboard widget.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return An array of [{key, value}] rows in display order.
     *   Variables the UPS does not report are skipped silently.
     */
    public function getInformation($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.nut");
        if (false === $object->get("enable")) {
            return [];
        }
        $vars = $this->parseVars($this->getUpscOutput($object));
        // Curated rows in display order: [variable => [label, unit]].
        // Reorder/extend freely; absent variables are skipped.
        $labels = [
            "device.mfr" => [gettext("Manufacturer"), ""],
            "device.model" => [gettext("Model"), ""],
            "battery.type" => [gettext("Battery type"), ""],
            "ups.status" => [gettext("Status"), ""],
            "input.voltage" => [gettext("Input voltage"), " V"],
            "ups.load" => [gettext("Load"), " %"],
            "battery.voltage" => [gettext("Battery voltage"), " V"],
            "battery.runtime" => [gettext("Runtime"), ""],
            "battery.mfr.date" => [gettext("Battery date"), ""],
            "battery.charge.low" => [gettext("Battery low"), " %"],
            "battery.charge.warning" => [gettext("Charge warning"), " %"],
            "battery.runtime.low" => [gettext("Runtime low"), ""]
        ];
        // These variables are rendered as a "<m>m<ss>s" duration.
        $durationKeys = ["battery.runtime", "battery.runtime.low"];
        $rows = [];
        foreach ($labels as $key => $def) {
            if (!array_key_exists($key, $vars)) {
                continue;
            }
            $value = in_array($key, $durationKeys)
                ? $this->formatDuration($vars[$key])
                : $vars[$key].$def[1];
            $rows[] = ["key" => $def[0], "value" => $value];
        }
        return $rows;
    }

    /**
     * Format a number of seconds as a "<minutes>m<seconds>s" string,
     * e.g. 1792 becomes "29m52s". Non-numeric input is returned as is.
     * @param value The number of seconds to format.
     * @return The formatted duration string.
     */
    private function formatDuration($value)
    {
        if (!is_numeric($value)) {
            return $value;
        }
        $seconds = (int) $value;
        return sprintf("%dm%02ds", intdiv($seconds, 60), $seconds % 60);
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
