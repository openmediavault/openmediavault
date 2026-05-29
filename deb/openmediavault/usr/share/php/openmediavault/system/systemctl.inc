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

namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * @ingroup api
 * Control the systemd system and service manager.
 */
class SystemCtl
{
    private $name = "";
    private $options = [];

    /**
     * Constructor
     * @param name The name of the unit, e.g. ssh.
     */
    public function __construct($name)
    {
        $this->name = $name;
    }

    /**
     * Get the name of the unit.
     * @return The name of the unit.
     */
    public function getName()
    {
        return $this->name;
    }

    /**
     * Set a command option.
     */
    public function setOption($arg)
    {
        $this->options[] = $arg;
    }

    /**
     * Reset the list of command options.
     */
    public function clearOptions()
    {
        $this->options = [];
    }

    /**
     * Execute the given command.
     * @param command The command to execute, e.g. start, stop, restart, ...
     * @param options A list of optional command options.
     * @param quiet Do not throw an error on failure. Defaults to FALSE.
     * @param output If the output argument is present, then the specified
     *   array will be filled with every line of output from the command.
     * @return The exit code if \em quite is set to TRUE.
     */
    private function exec(
        $command,
        array $options = null,
        $quiet = false,
        array &$output = null
    ) {
        $cmdArgs = [];
        // Append (global) command options.
        if (!empty($this->options) && is_array($this->options)) {
            $cmdArgs = array_merge($cmdArgs, $this->options);
        }
        // Append optional command options.
        if (!empty($options) && is_array($options)) {
            $cmdArgs = array_merge($cmdArgs, $options);
        }
        $cmdArgs[] = $command;
        $cmdArgs[] = escapeshellarg($this->getName());
        // Execute the command.
        $cmd = new \OMV\System\Process("systemctl", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->setQuiet($quiet);
        $cmd->execute($output, $exitStatus);
        return $exitStatus;
    }

    /**
     * Enable the specified unit file.
     * @param now Set to TRUE to start the unit after it has been enabled.
     *   Defaults to FALSE.
     */
    public function enable($now = false, $quiet = false)
    {
        $options = [];
        if (true === $now) {
            $options[] = "--now";
        }
        $this->exec("enable", $options, $quiet);
    }

    /**
     * Disable the specified unit file.
     * @param now Set to TRUE to stop the unit after it has been disabled.
     *   Defaults to FALSE.
     */
    public function disable($now = false, $quiet = false)
    {
        $options = [];
        if (true === $now) {
            $options[] = "--now";
        }
        $this->exec("disable", $options, $quiet);
    }

    /**
     * Reenable the specified unit file.
     */
    public function reenable($quiet = false)
    {
        $this->exec("reenable", null, $quiet);
    }

    /**
     * Checks whether the specified unit file is enabled.
     * @return TRUE if the unit is enabled, otherwise FALSE.
     */
    public function isEnabled()
    {
        $exitStatus = $this->exec("is-enabled", null, true, $output);
        if (0 !== $exitStatus) {
            return false;
        }
        return "enabled" == trim($output[0]);
    }

    /**
     * Start (activate) the specified unit.
     */
    public function start($quiet = false)
    {
        $this->exec("start", null, $quiet);
    }

    /**
     * Stop (deactivate) the specified unit.
     */
    public function stop($quiet = false)
    {
        $this->exec("stop", null, $quiet);
    }

    /**
     * Restart the specified unit.
     */
    public function restart($quiet = false)
    {
        $this->exec("restart", null, $quiet);
    }

    /**
     * Asks the specified unit to reload its configuration.
     */
    public function reload($quiet = false)
    {
        $this->exec("reload", null, $quiet);
    }

    /**
     * Checks whether the specified unit file is masked.
     * @return TRUE if the unit is masked, otherwise FALSE.
     */
    public function isMasked(): bool
    {
        $exitStatus = $this->exec("is-enabled", null, true, $output);
        if (0 == $exitStatus) {
            return false;
        }
        return "masked" == trim($output[0]);
    }

    /**
     * Mask the specified unit file.
     */
    public function mask($quiet = false)
    {
        $this->exec("mask", null, $quiet);
    }

    /**
     * Unmask the specified unit file.
     */
    public function unmask($quiet = false)
    {
        $this->exec("unmask", null, $quiet);
    }

    /**
     * Check whether the specified unit is active (i.e. running).
     * @return TRUE if the unit is active, otherwise FALSE.
     */
    public function isActive(): bool
    {
        return (0 == $this->exec("is-active", null, true));
    }

    /**
     * Wait until the specified unit is active. If the unit is not active
     * within the given time, then an exception is thrown.
     * @param timeout Timeout in seconds to wait for an active unit.
     * @return void
     * @throw \OMV\Exception
     */
    public function waitUntilActive($timeout): void
    {
        $message = sprintf(
            "The unit '%s' is not active after a waiting period of %d seconds.",
            $this->getName(),
            $timeout
        );
        waitUntil($timeout, [$this, "isActive"], $message);
    }

    /**
     * Send a signal to the process of the unit.
     * @param signal Must be one of the well known signal specifiers such as
     *   SIGHUP, SIGTERM, SIGINT or SIGSTOP.
     */
    public function kill($signal, $quiet = false)
    {
        $options = [];
        $options[] = sprintf("--signal=%s", $signal);
        return (0 == $this->exec("kill", $options, $quiet));
    }

    /**
     * Escape strings for usage in system unit names.
     * @see https://www.freedesktop.org/software/systemd/man/systemd-escape.html
     * @param str The string to escape.
     * @param options An array of 'systemd-escape' options.
     * @return Returns the escaped string.
     * @throw \OMV\Exception
     */
    public static function escape($str, $options)
    {
        $cmdArgs = [];
        if (!empty($options) && is_array($options)) {
            $cmdArgs = array_merge($cmdArgs, $options);
        }
        $cmdArgs[] = escapeshellarg($str);
        $cmd = new \OMV\System\Process("systemd-escape", $cmdArgs);
        $cmd->setRedirect2to1();
        return $cmd->execute();
    }
}
