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
require_once("openmediavault/functions.inc");

class StorageDeviceStub {
	private $deviceFile;
	private $wwn;

	public function __construct(string $deviceFile = "/dev/sda",
			string $wwn = "") {
		$this->deviceFile = $deviceFile;
		$this->wwn = $wwn;
	}

	public function assertHasSmartSupport(): void {}

	public function getDeviceFile(): string {
		return $this->deviceFile;
	}

	public function getWorldWideName(): string {
		return $this->wwn;
	}

	public function getSmartDeviceType(): string {
		return "";
	}
}

class FakeSmartInformation extends \OMV\System\Storage\SmartInformation {
	public function __construct(array $cmdOutput,
			string $deviceFile = "/dev/sda", string $wwn = "") {
		parent::__construct(new StorageDeviceStub($deviceFile, $wwn));
		$ref = new \ReflectionClass(parent::class);
		$prop = $ref->getProperty('cmdOutput');
		$prop->setAccessible(TRUE);
		$prop->setValue($this, $cmdOutput);
		$cached = $ref->getProperty('dataCached');
		$cached->setAccessible(TRUE);
		$cached->setValue($this, TRUE);
		$cachedKind = $ref->getProperty('dataCachedKind');
		$cachedKind->setAccessible(TRUE);
		$cachedKind->setValue($this, \OMV\System\Storage\SmartInformation::SMART_INFO_KIND_DEFAULT);
	}
}

class test_openmediavault_system_storage_smartinformation
		extends \PHPUnit\Framework\TestCase {

	private function getAtaOutput(): array {
		return [
			"smartctl 7.4 2023-08-01 r5530 [x86_64-linux-6.19.10+deb13-amd64] (local build)",
			"Copyright (C) 2002-23, Bruce Allen, Christian Franke, www.smartmontools.org",
			"",
			"=== START OF INFORMATION SECTION ===",
			"Model Family:     Western Digital Gold",
			"Device Model:     WDC WD8004FRYZ-01VAEB0",
			"Serial Number:    VGK6VV1G",
			"LU WWN Device Id: 5 000cca 0beed657e",
			"Firmware Version: 01.01H01",
			"User Capacity:    8,001,563,222,016 bytes [8.00 TB]",
			"Sector Sizes:     512 bytes logical, 4096 bytes physical",
			"Rotation Rate:    7200 rpm",
			"Form Factor:      3.5 inches",
			"Device is:        In smartctl database 7.3/6114",
			"ATA Version is:   ACS-2, ATA8-ACS T13/1699-D revision 4",
			"SATA Version is:  SATA 3.2, 6.0 Gb/s (current: 6.0 Gb/s)",
			"Local Time is:    Sun Apr 12 15:17:10 2026 EEST",
			"SMART support is: Available - device has SMART capability.",
			"SMART support is: Enabled",
			"",
			"=== START OF READ SMART DATA SECTION ===",
			"SMART Status not supported: Incomplete response, ATA output registers missing",
			"SMART overall-health self-assessment test result: PASSED",
			"Warning: This result is based on an Attribute check.",
			"",
			"SMART Attributes Data Structure revision number: 16",
			"Vendor Specific SMART Attributes with Thresholds:",
			"ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE",
			"  5 Reallocated_Sector_Ct   0x0033   100   100   005    Pre-fail  Always       -       0",
			"  9 Power_On_Hours          0x0012   099   099   000    Old_age   Always       -       12466",
			" 12 Power_Cycle_Count       0x0032   100   100   000    Old_age   Always       -       211",
			"194 Temperature_Celsius     0x0002   096   096   000    Old_age   Always       -       62 (Min/Max 5/62)",
			"197 Current_Pending_Sector  0x0022   100   100   000    Old_age   Always       -       0",
			"",
			"SMART Self-test log structure revision number 1",
			"Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error",
			"# 1  Short offline       Completed without error       00%     12440         -",
		];
	}

	private function getNvmeOutput(): array {
		return [
			"smartctl 7.3 2022-02-28 r5338 [x86_64-linux-6.1.0] (local build)",
			"",
			"=== START OF INFORMATION SECTION ===",
			"Model Number:     Samsung SSD 960 PRO 1TB",
			"Serial Number:    S3EVNX0J603076T",
			"Firmware Version: 2B6QCXP7",
			"PCI Vendor/Subsystem ID: 0x144d",
			"IEEE OUI Identifier: 0x002538",
			"Total NVM Capacity: 1,024,209,543,168 [1.02 TB]",
			"Namespace 1 Size/Capacity: 1,024,209,543,168 [1.02 TB]",
			"Local Time is: Fri Apr 24 07:24:03 2026 CDT",
			"",
			"=== START OF SMART DATA SECTION ===",
			"SMART overall-health self-assessment test result: PASSED",
			"",
			"SMART/Health Information (NVMe Log 0x02)",
			"Critical Warning:                   0x00",
			"Temperature:                        36 Celsius",
			"Available Spare:                    100%",
			"Available Spare Threshold:          10%",
			"Percentage Used:                    7%",
			"Data Units Read:                    125,056,648 [64.0 TB]",
			"Data Units Written:                 110,753,351 [56.7 TB]",
			"Host Read Commands:                 1,055,460,163",
			"Host Write Commands:                3,032,068,159",
			"Controller Busy Time:               14,611",
			"Power Cycles:                       31",
			"Power On Hours:                     29,685",
			"Unsafe Shutdowns:                   21",
			"Media and Data Integrity Errors:    0",
			"Error Information Log Entries:      51",
			"Warning  Comp. Temperature Time:    0",
			"Critical Comp. Temperature Time:    0",
			"Temperature Sensor 1:               36 Celsius",
			"Temperature Sensor 2:               46 Celsius",
		];
	}

	private function getSasOutput(): array {
		return [
			"smartctl 7.4 2024-10-15 r5620 [x86_64-linux-6.19.10+deb13-amd64] (local build)",
			"Copyright (C) 2002-23, Bruce Allen, Christian Franke, www.smartmontools.org",
			"",
			"=== START OF INFORMATION SECTION ===",
			"Vendor:               HITACHI",
			"Product:              H106060SDSUN600G",
			"Revision:             A4C0",
			"Compliance:           SPC-4",
			"User Capacity:        600,127,266,816 bytes [600 GB]",
			"Logical block size:   512 bytes",
			"Rotation Rate:        10020 rpm",
			"Form Factor:          2.5 inches",
			"Logical Unit id:      0x5000cca03c587e1c",
			"Serial number:        001650PKNPTD        PZHKNPTD",
			"Device type:          disk",
			"Transport protocol:   SAS (SPL-4)",
			"Local Time is:        Mon Apr 13 16:24:27 2026 EDT",
			"SMART support is:     Available - device has SMART capability.",
			"SMART support is:     Enabled",
			"Temperature Warning:  Enabled",
			"",
			"=== START OF READ SMART DATA SECTION ===",
			"SMART Health Status: OK",
			"",
			"Current Drive Temperature:     28 C",
			"Drive Trip Temperature:        85 C",
			"",
			"Accumulated power on time, hours:minutes 55120:47",
			"Manufactured in week 50 of year 2016",
			"Specified cycle count over device lifetime:  50000",
			"Accumulated start-stop cycles:  34",
			"Specified load-unload count over device lifetime:  300000",
			"Accumulated load-unload cycles:  2306",
			"Elements in grown defect list: 0",
			"",
			"Vendor (Seagate Cache) information",
			"  Blocks sent to initiator = 3406564736106496",
		];
	}

	public function testAtaDeviceModel(): void {
		$si = new FakeSmartInformation($this->getAtaOutput());
		$info = $si->getInformation();
		$this->assertSame("WDC WD8004FRYZ-01VAEB0", $info['devicemodel']);
	}

	public function testAtaSerialNumber(): void {
		$si = new FakeSmartInformation($this->getAtaOutput());
		$info = $si->getInformation();
		$this->assertSame("VGK6VV1G", $info['serialnumber']);
	}

	public function testAtaFirmwareVersion(): void {
		$si = new FakeSmartInformation($this->getAtaOutput());
		$info = $si->getInformation();
		$this->assertSame("01.01H01", $info['firmwareversion']);
	}

	public function testAtaUserCapacity(): void {
		$si = new FakeSmartInformation($this->getAtaOutput());
		$info = $si->getInformation();
		$this->assertSame("8,001,563,222,016 bytes [8.00 TB]", $info['usercapacity']);
	}

	public function testAtaPowerModeUnknownWhenNotPresent(): void {
		$si = new FakeSmartInformation($this->getAtaOutput());
		$this->assertSame("UNKNOWN", $si->getPowerMode());
	}

	public function testNvmeDeviceModel(): void {
		$si = new FakeSmartInformation($this->getNvmeOutput(),
			"/dev/disk/by-path/pci-0000:02:00.0-nvme-1",
			"eui.0025385671b068e7");
		$info = $si->getInformation();
		$this->assertSame("Samsung SSD 960 PRO 1TB", $info['devicemodel']);
	}

	public function testNvmeSerialNumber(): void {
		$si = new FakeSmartInformation($this->getNvmeOutput());
		$info = $si->getInformation();
		$this->assertSame("S3EVNX0J603076T", $info['serialnumber']);
	}

	public function testNvmeFirmwareVersion(): void {
		$si = new FakeSmartInformation($this->getNvmeOutput());
		$info = $si->getInformation();
		$this->assertSame("2B6QCXP7", $info['firmwareversion']);
	}

	public function testNvmeTemperatureKey(): void {
		$si = new FakeSmartInformation($this->getNvmeOutput());
		$info = $si->getInformation();
		$this->assertArrayHasKey('temperature', $info);
		$this->assertSame("36 Celsius", $info['temperature']);
	}

	public function testNvmePowerCycles(): void {
		$si = new FakeSmartInformation($this->getNvmeOutput());
		$info = $si->getInformation();
		$this->assertArrayHasKey('powercycles', $info);
		$this->assertSame("31", $info['powercycles']);
	}

	public function testNvmePowerOnHours(): void {
		$si = new FakeSmartInformation($this->getNvmeOutput());
		$info = $si->getInformation();
		$this->assertArrayHasKey('poweronhours', $info);
		$this->assertSame("29,685", $info['poweronhours']);
	}

	public function testNvmeHealthAssessmentGood(): void {
		$si = new FakeSmartInformation($this->getNvmeOutput());
		$this->assertSame(
			\OMV\System\Storage\SmartInformation::SMART_ASSESSMENT_GOOD,
			$si->getOverallStatus()
		);
	}

	public function testSasVendor(): void {
		$si = new FakeSmartInformation($this->getSasOutput());
		$info = $si->getInformation();
		$this->assertSame("HITACHI", $info['vendor']);
	}

	public function testSasProduct(): void {
		$si = new FakeSmartInformation($this->getSasOutput());
		$info = $si->getInformation();
		$this->assertSame("H106060SDSUN600G", $info['product']);
	}

	public function testSasFirmwareVersionFromRevision(): void {
		$si = new FakeSmartInformation($this->getSasOutput());
		$info = $si->getInformation();
		$this->assertSame("A4C0", $info['firmwareversion']);
		$this->assertSame($info['firmwareversion'], $info['revision']);
	}

	public function testSasUserCapacity(): void {
		$si = new FakeSmartInformation($this->getSasOutput());
		$info = $si->getInformation();
		$this->assertSame("600,127,266,816 bytes [600 GB]", $info['usercapacity']);
	}

	public function testSasCurrentDriveTemperature(): void {
		$si = new FakeSmartInformation($this->getSasOutput());
		$info = $si->getInformation();
		$this->assertArrayHasKey('currentdrivetemperature', $info);
		$this->assertSame("28 C", $info['currentdrivetemperature']);
	}

	public function testSasAccumulatedStartStopCycles(): void {
		$si = new FakeSmartInformation($this->getSasOutput());
		$info = $si->getInformation();
		$this->assertArrayHasKey('accumulatedstartstopcycles', $info);
		$this->assertSame("34", $info['accumulatedstartstopcycles']);
	}

	public function testSasHealthAssessmentGood(): void {
		$si = new FakeSmartInformation($this->getSasOutput());
		$this->assertSame(
			\OMV\System\Storage\SmartInformation::SMART_ASSESSMENT_GOOD,
			$si->getOverallStatus()
		);
	}

	public function testMultipleSectionsAreParsed(): void {
		$output = [
			"=== START OF INFORMATION SECTION ===",
			"Device Model:     TestDrive",
			"Serial Number:    SN123",
			"",
			"=== START OF SMART DATA SECTION ===",
			"Temperature:      45 Celsius",
		];
		$si = new FakeSmartInformation($output);
		$info = $si->getInformation();
		$this->assertSame("TestDrive", $info['devicemodel']);
		$this->assertSame("SN123", $info['serialnumber']);
		$this->assertSame("45 Celsius", $info['temperature']);
	}

	public function testGetTemperatureFromAttrId194(): void {
		$output = [
			"=== START OF INFORMATION SECTION ===",
			"Device Model:     TestDrive",
			"Serial Number:    SN123",
			"",
			"=== START OF READ SMART DATA SECTION ===",
			"ID# ATTRIBUTE_NAME          FLAGS    VALUE WORST THRESH FAIL RAW_VALUE",
			"194 Temperature_Celsius     -O---K   076   037   ---    -    36 (Min/Max 19/37)",
		];
		$si = new FakeSmartInformation($output);
		$this->assertSame("36", $si->getTemperature());
	}

	public function testGetTemperatureFromAttrId190(): void {
		$output = [
			"=== START OF READ SMART DATA SECTION ===",
			"ID# ATTRIBUTE_NAME          FLAGS    VALUE WORST THRESH FAIL RAW_VALUE",
			"190 Airflow_Temperature_Cel -O---K   065   044   045    Past 35 (0 3 35 35 0)",
		];
		$si = new FakeSmartInformation($output);
		$this->assertSame("35", $si->getTemperature());
	}

	public function testGetTemperatureFromCurrentTemperatureLine(): void {
		$output = [
			"=== START OF READ SMART DATA SECTION ===",
			"Current Temperature:                    28 Celsius",
		];
		$si = new FakeSmartInformation($output);
		$this->assertSame("28", $si->getTemperature());
	}

	public function testGetTemperatureReturnsFalseWhenNotPresent(): void {
		$si = new FakeSmartInformation($this->getAtaOutput());
		$this->assertFalse($si->getTemperature());
	}

	public function testGetTemperatureNvme(): void {
		$si = new FakeSmartInformation($this->getNvmeOutput());
		$this->assertSame("36", $si->getTemperature());
	}

	public function testGetTemperatureSas(): void {
		$si = new FakeSmartInformation($this->getSasOutput());
		$this->assertSame("28", $si->getTemperature());
	}
}

