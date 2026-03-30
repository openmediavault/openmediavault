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

class test_openmediavault_config_apt_schedule extends \PHPUnit\Framework\TestCase {
    protected function setUp(): void {
        // Initialize test environment
        $this->config = new \OMV\Config\Database();
    }

    public function testAptUpdatesScheduleValidation() {
        // Test valid schedule values
        $validSchedules = ['daily', 'weekly', 'monthly'];
        $validConfig = [
            'unattendedupgrade' => true,
            'schedule' => 'weekly'
        ];
        
        $schema = \OMV\DataModel\Schema::get("conf.system.apt.updates");
        $this->assertInstanceOf('\OMV\DataModel\Schema', $schema);
        
        foreach ($validSchedules as $schedule) {
            $config = $validConfig;
            $config['schedule'] = $schedule;
            
            // Test that valid values pass validation
            $this->assertNull($schema->validate($config));
        }
    }

    public function testAptUpdatesScheduleInvalidValue() {
        // Test invalid schedule values
        $invalidSchedules = ['hourly', 'yearly', 'biweekly', ''];
        
        $schema = \OMV\DataModel\Schema::get("conf.system.apt.updates");
        $this->assertInstanceOf('\OMV\DataModel\Schema', $schema);
        
        foreach ($invalidSchedules as $schedule) {
            $config = [
                'unattendedupgrade' => true,
                'schedule' => $schedule
            ];
            
            // Test that invalid values fail validation
            $this->expectException(\OMV\DataModel\ValidationException::class);
            $schema->validate($config);
        }
    }

    public function testAptUpdatesDefaultSchedule() {
        // Test that default schedule is daily
        $schema = \OMV\DataModel\Schema::get("conf.system.apt.updates");
        $this->assertInstanceOf('\OMV\DataModel\Schema', $schema);
        
        $config = [
            'unattendedupgrade' => true
        ];
        
        // Test that the configuration without schedule defaults to daily
        $schema->validate($config);
        $this->assertEquals('daily', $config['schedule']);
    }

    public function testAptUpdatesUnattendedUpgrade() {
        // Test unattendedupgrade property
        $schema = \OMV\DataModel\Schema::get("conf.system.apt.updates");
        $this->assertInstanceOf('\OMV\DataModel\Schema', $schema);
        
        $validConfig = [
            'unattendedupgrade' => false,
            'schedule' => 'daily'
        ];
        
        // Test that boolean values work correctly
        $schema->validate($validConfig);
        $this->assertFalse($validConfig['unattendedupgrade']);
    }
    
    public function testScheduleConfigurationPersistence() {
        // Test that we can store and retrieve schedule configuration
        $config = new \OMV\Config\Database();
        $this->assertInstanceOf('\OMV\Config\Database', $config);
        
        // Create a test configuration object
        $object = [
            'unattendedupgrade' => true,
            'schedule' => 'monthly'
        ];
        
        $result = $config->set("conf.system.apt.updates", $object);
        $this->assertNotNull($result);
        
        // Retrieve the configuration
        $retrieved = $config->get("conf.system.apt.updates");
        $this->assertNotNull($retrieved);
        $this->assertEquals('monthly', $retrieved['schedule']);
        $this->assertTrue($retrieved['unattendedupgrade']);
    }
    
    public function testScheduleDefaultValuePersistence() {
        // Test that when no schedule is specified, default is used
        $config = new \OMV\Config\Database();
        $this->assertInstanceOf('\OMV\Config\Database', $config);
        
        // Create a test configuration object without schedule
        $object = [
            'unattendedupgrade' => false
        ];
        
        $result = $config->set("conf.system.apt.updates", $object);
        $this->assertNotNull($result);
        
        // Retrieve the configuration
        $retrieved = $config->get("conf.system.apt.updates");
        $this->assertNotNull($retrieved);
        $this->assertEquals('daily', $retrieved['schedule']); // Should default to daily
        $this->assertFalse($retrieved['unattendedupgrade']);
    }
}