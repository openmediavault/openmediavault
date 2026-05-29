#!/usr/bin/phpunit -c/etc/openmediavault
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
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/globals.inc");

class test_openmediavault_system_systemctrl extends \PHPUnit\Framework\TestCase
{
    private static function isGithubCI(): bool
    {
        return ("true" === getenv("GITHUB_ACTIONS"));
    }

    private function skipIfSystemdUnavailable(): void
    {
        if (!is_dir("/run/systemd/system")) {
            $this->markTestSkipped("systemd is not available in this environment.");
        }
    }

    private function skipIfNoRootForMutatingSystemctlTests(): void
    {
        if (!function_exists("posix_geteuid") || (0 !== posix_geteuid())) {
            $this->markTestSkipped("Mutating systemctl tests require root privileges.");
        }
    }

    private function skipIfSystemdEscapeUnavailable(): void
    {
        if (!is_executable("/usr/bin/systemd-escape") &&
            !is_executable("/bin/systemd-escape")) {
            $this->markTestSkipped("systemd-escape is not available in this environment.");
        }
    }

    public function testGetName(): void
    {
        $systemCtl = new \OMV\System\SystemCtl("dummy.service");
        $this->assertEquals("dummy.service", $systemCtl->getName());
    }

    public function testSetAndClearOptions(): void
    {
        $systemCtl = new \OMV\System\SystemCtl("dummy.service");
        $systemCtl->setOption("--foo");
        $systemCtl->setOption("--bar");
        $reflection = new \ReflectionClass($systemCtl);
        $property = $reflection->getProperty("options");
        $property->setAccessible(true);
        $this->assertEquals(["--foo", "--bar"], $property->getValue($systemCtl));
        $systemCtl->clearOptions();
        $this->assertEquals([], $property->getValue($systemCtl));
    }

    public function testEscapePath(): void
    {
        $this->skipIfSystemdEscapeUnavailable();
        $this->assertEquals(
            "srv-shares-test",
            \OMV\System\SystemCtl::escape("/srv/shares/test", ["--path"])
        );
    }

    public function testEscapeTemplateInstance(): void
    {
        $this->skipIfSystemdEscapeUnavailable();
        $this->assertEquals(
            "foo@bar.service",
            \OMV\System\SystemCtl::escape("bar", ["--template", "foo@.service"])
        );
    }

    public function testIsEnabled()
    {
        $this->skipIfSystemdUnavailable();
        if (self::isGitHubCI()) {
            $this->markTestSkipped("Skipped on GitHub CI because rsyslog.service may be unavailable.");
        }
        $systemCtl = new \OMV\System\SystemCtl("rsyslog.service");
        $this->assertTrue($systemCtl->isEnabled());
    }

    public function testIsNotEnabled()
    {
        $this->skipIfSystemdUnavailable();
        $systemCtl = new \OMV\System\SystemCtl("ctrl-alt-del.target");
        $this->assertFalse($systemCtl->isEnabled());
    }

    public function testIsActive()
    {
        $this->skipIfSystemdUnavailable();
        if (self::isGitHubCI()) {
            $this->markTestSkipped("Skipped on GitHub CI because rsyslog.service may be unavailable.");
        }
        $systemCtl = new \OMV\System\SystemCtl("rsyslog.service");
        $this->assertTrue($systemCtl->isActive());
    }

    public function testEnableDisable()
    {
        $this->skipIfSystemdUnavailable();
        $this->skipIfNoRootForMutatingSystemctlTests();
        if (self::isGitHubCI()) {
            $this->markTestSkipped("Skipped on GitHub CI because mutating system services is unsafe.");
        }
        $systemCtl = new \OMV\System\SystemCtl("chrony.service");
        $isEnabled = $systemCtl->isEnabled();
        $isActive = $systemCtl->isActive();
        $systemCtl->enable(true);
        $this->assertTrue($systemCtl->isEnabled());
        $this->assertTrue($systemCtl->isActive());
        $systemCtl->disable();
        $this->assertFalse($systemCtl->isEnabled());
        $this->assertTrue($systemCtl->isActive());
        $systemCtl->disable(true);
        $this->assertFalse($systemCtl->isEnabled());
        $this->assertFalse($systemCtl->isActive());
        if ($isEnabled) {
            $systemCtl->enable($isActive);
            $this->assertTrue($systemCtl->isEnabled());
            $this->assertTrue($systemCtl->isActive());
        }
    }

    public function testIsMasked()
    {
        $this->skipIfSystemdUnavailable();
        if (self::isGitHubCI()) {
            $this->markTestSkipped("Skipped on GitHub CI because hwclock.service may be unavailable.");
        }
        $systemCtl = new \OMV\System\SystemCtl("hwclock.service");
        $this->assertTrue($systemCtl->isMasked());
    }

    public function testIsNotMasked()
    {
        $this->skipIfSystemdUnavailable();
        $systemCtl = new \OMV\System\SystemCtl("default.target");
        $this->assertFalse($systemCtl->isMasked());
    }

    public function testMaskUnmask()
    {
        $this->skipIfSystemdUnavailable();
        $this->skipIfNoRootForMutatingSystemctlTests();
        if (self::isGitHubCI()) {
            $this->markTestSkipped("Skipped on GitHub CI because mutating system services is unsafe.");
        }
        $systemCtl = new \OMV\System\SystemCtl("chrony.service");
        $isMasked = $systemCtl->isMasked();
        $systemCtl->mask();
        $this->assertTrue($systemCtl->isMasked());
        $systemCtl->unmask();
        $this->assertFalse($systemCtl->isMasked());
        if ($isMasked) {
            $systemCtl->mask();
            $this->assertTrue($systemCtl->isMasked());
        }
    }
}
