#!/usr/bin/phpunit -c/etc/openmediavault
<?php

/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Karol Olszacki <karol@olszacki.pl>
 * @copyright Copyright (c) 2026 Karol Olszacki
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
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/globals.inc");

class test_openmediavault_system_process extends \PHPUnit\Framework\TestCase
{
    /**
     * The command line must not be modified if no timeout is configured.
     */
    public function testNoTimeoutByDefault()
    {
        $cmd = new \OMV\System\Process("blkid", "-o", "full");
        $cmdLine = $cmd->getCommandLine();
        $this->assertStringNotContainsString("timeout", $cmdLine);
        $this->assertStringContainsString("blkid -o full", $cmdLine);
    }

    /**
     * A configured timeout must wrap the command, but it has to be placed
     * after the environment exports and before the command itself.
     */
    public function testTimeoutWrapsCommand()
    {
        $cmd = new \OMV\System\Process("blkid", "-o", "full");
        $cmd->setTimeout(30);
        $cmdLine = $cmd->getCommandLine();
        $this->assertStringContainsString("timeout --kill-after=5 30", $cmdLine);
        $this->assertStringContainsString("timeout --kill-after=5 30 blkid -o full", $cmdLine);
        // The exports must stay in front of the timeout, otherwise the
        // timeout would be executed with the wrong environment.
        $this->assertMatchesRegularExpression(
            "/^export .*; timeout --kill-after=5 30 blkid -o full$/",
            $cmdLine
        );
    }

    /**
     * A timeout of zero disables the limit again.
     */
    public function testTimeoutCanBeDisabled()
    {
        $cmd = new \OMV\System\Process("blkid", "-o", "full");
        $cmd->setTimeout(30);
        $cmd->setTimeout(0);
        $this->assertStringNotContainsString("timeout", $cmd->getCommandLine());
    }

    /**
     * The exit statuses of a command that was aborted by the timeout have
     * to be recognisable by name, they are not arbitrary numbers.
     */
    public function testTimeoutExitStatusDetection()
    {
        $this->assertTrue(
            \OMV\System\Process::isTimeoutExitStatus(
                \OMV\System\Process::EXIT_TIMEOUT
            )
        );
        $this->assertTrue(
            \OMV\System\Process::isTimeoutExitStatus(
                \OMV\System\Process::EXIT_KILLED
            )
        );
        $this->assertFalse(\OMV\System\Process::isTimeoutExitStatus(0));
        $this->assertFalse(\OMV\System\Process::isTimeoutExitStatus(2));
        $this->assertFalse(\OMV\System\Process::isTimeoutExitStatus(1));
    }
}
