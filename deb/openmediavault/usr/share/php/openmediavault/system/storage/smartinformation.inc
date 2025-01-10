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
namespace OMV\System\Storage;

/**
 * This class implements methods to get and process S.M.A.R.T.
 * AT Attachment (ATA) information and properties.
 * @ingroup api
 */
class SmartInformation {
	protected $sd = NULL;
	protected $noCheck = NULL;
	private $cmdOutput = NULL;
	private $dataCached = FALSE;
	private $dataCachedKind = NULL;
	private $attrDesc = [
		1 => "Frequency of errors while reading raw data from the disk",
		2 => "Average efficiency of the disk",
		3 => "Time needed to spin up the disk",
		4 => "Number of spindle start/stop cycles",
		5 => "Number of remapped sectors",
		6 => "Margin of a channel while reading data",
		7 => "Frequency of errors while positioning",
		8 => "Average efficiency of operations while positioning",
		9 => "Number of hours elapsed in the power-on state",
		10 => "Number of retry attempts to spin up",
		11 => "Number of attempts to calibrate the device",
		12 => "Number of power-on events",
		13 => "Frequency of errors while reading from the disk",
		173 => "Counts the maximum worst erase count on any block",
		187 => "Number of errors that could not be recovered using hardware ECC",
		188 => "The count of aborted operations due to HDD timeout",
		189 => "Number of times a recording head is flying outside its normal operating range",
		190 => "Airflow temperature of the drive",
		191 => "Frequency of mistakes as a result of impact loads",
		192 => "Number of power-off or emergency retract cycles",
		193 => "Number of cycles into landing zone position",
		194 => "Current internal temperature of the drive",
		195 => "Number of ECC on-the-fly errors",
		196 => "Number of remapping operations",
		197 => "Number of sectors waiting to be remapped",
		198 => "The total number of uncorrectable errors when reading/writing a sector",
		199 => "Number of CRC errors during UDMA mode",
		200 => "Number of errors found when writing a sector",
		201 => "Number of off-track errors",
		203 => "Number of ECC errors",
		204 => "Number of errors corrected by software ECC",
		205 => "Number of errors due to high temperature",
		206 => "Height of heads above the disk surface",
		207 => "Amount of high current used to spin up the drive",
		208 => "Number of buzz routines to spin up the drive",
		209 => "Drive’s seek performance during offline operations",
		220 => "Distance the disk has shifted relative to the spindle",
		221 => "Number of errors as a result of impact loads as detected by a shock sensor",
		222 => "Number of hours in general operational state",
		223 => "Number of times head changes position",
		224 => "Load on drive caused by friction in mechanical parts of the store",
		225 => "Total number of load cycles",
		226 => "General time for loading in a drive",
		227 => "Number of attempts to compensate for platter speed variations",
		228 => "Number of power-off retract events",
		230 => "Amplitude of heads trembling in running mode",
		231 => "Temperature of the drive",
		232 => "Number of physical erase cycles completed on the drive as a percentage of the maximum physical erase cycles the drive supports",
		233 => "Number of hours elapsed in the power-on state",
		235 => "Number of available reserved blocks as a percentage of the total number of reserved blocks",
		240 => "Time spent during the positioning of the drive heads",
		250 => "Number of errors while reading from a disk",
		251 => "Number of remaining spare blocks as a percentage of the total number of spare blocks available",
		252 => "Total number of bad flash blocks the drive detected since it was first initialized in manufacturing",
		254 => "Number of detected free fall events"
	];

	const SMART_ASSESSMENT_GOOD = "GOOD";
	const SMART_ASSESSMENT_BAD_ATTRIBUTE_IN_THE_PAST = "BAD_ATTRIBUTE_IN_THE_PAST";
	const SMART_ASSESSMENT_BAD_SECTOR = "BAD_SECTOR";
	const SMART_ASSESSMENT_BAD_ATTRIBUTE_NOW = "BAD_ATTRIBUTE_NOW";
	const SMART_ASSESSMENT_BAD_SECTOR_MANY = "BAD_SECTOR_MANY";
	const SMART_ASSESSMENT_BAD_STATUS = "BAD_STATUS";

	const SMART_INFO_KIND_DEFAULT = "default";
	const SMART_INFO_KIND_ALL = "all";

	/**
	 * Constructor
	 * @param sd The storage device object. Defaults to NULL.
	 */
	public function __construct($sd) {
		$sd->assertHasSmartSupport();
		$this->sd = $sd;
	}

	/**
	 * Get S.M.A.R.T. information.
	 * @private
	 * @oaram string $kind The kind of SMART information. If set to
	 *   `all`, SMART and non-SMART information are requested.
	 *   Defaults to NULL (=> `default`) which requests only SMART
	 *   information.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	private function getData($kind = NULL) {
		if (is_null($kind)) {
			$kind = self::SMART_INFO_KIND_DEFAULT;
		}

		if (FALSE !== $this->isCached()) {
			if ($kind === $this->dataCachedKind) {
				return;
			}
			if (self::SMART_INFO_KIND_ALL === $this->dataCachedKind &&
					self::SMART_INFO_KIND_DEFAULT === $kind) {
				return;
			}
		}

		// Deny the parallel execution of the smartctl command for
		// this device. This is necessary to prevent an increasing
		// of the ATA error count of the device.
		$mutex = new \OMV\Mutex(__CLASS__.__METHOD__.$this->sd->getDeviceFile());

		// Get the SMART + non-SMART information?
		$cmdArgs = [];
		if (self::SMART_INFO_KIND_ALL === $kind) {
			$cmdArgs[] = "--xall";
		} else {
			$cmdArgs[] = "--attributes";
			$cmdArgs[] = "--format=brief";
			$cmdArgs[] = "--info";
			$cmdArgs[] = "--health";
			$cmdArgs[] = "--log=selftest";
		}
		if (!is_null($this->noCheck)) {
			$cmdArgs[] = sprintf("--nocheck=%s", escapeshellarg(
				$this->noCheck));
		}
		if (!empty($this->sd->getSmartDeviceType())) {
			$cmdArgs[] = sprintf("--device=%s", $this->sd->getSmartDeviceType());
		}
		$cmdArgs[] = $this->sd->getDeviceFile();
		$cmd = new \OMV\System\Process("smartctl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		// Check the exit status:
		// Bit 0: Command line did not parse
		// Bit 1: Device open failed, or device did not return an
		//        IDENTIFY DEVICE structure
		if (($exitStatus !== 0) && (($exitStatus & 0) || ($exitStatus & 1))) {
			throw new \OMV\ExecException($cmd, $output, $exitStatus);
		}
		$this->cmdOutput = $output;

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;
		$this->dataCachedKind = $kind;
	}

	/**
	 * Check if the SMART information is cached.
	 * @return boolean Returns TRUE if the SMART information is cached,
	 *   otherwise FALSE.
	 */
	public function isCached() {
		return $this->dataCached;
	}

	/**
	 * Refresh the cached information.
	 * @return void
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		$this->getData($this->dataCachedKind);
	}

	/**
	 * Set the `smartctl` `--noCheck` option.
	 * @see https://www.smartmontools.org/browser/trunk/smartmontools/smartctl.8.in#n
	 * @param string $noCheck The `--nocheck` option, e.g. `never`, `sleep`,
	 *   `standby`, `standby,0` or `idle`. Defaults to NULL.
	 * @return void
	 */
	public function setNoCheck($noCheck = NULL) {
		$this->noCheck = $noCheck;
	}

	/**
	 * Get the plain text 'smartctl' command output.
	 * @return string The command output.
	 */
	public function getExtendedInformation() {
		$this->getData(self::SMART_INFO_KIND_ALL);
		return implode("\n", $this->cmdOutput);
	}

	/**
	 * Get the S.M.A.R.T. attributes data structure.
	 * @return array An array of S.M.A.R.T. attributes data.
	 */
	public function getAttributes(): array {
		$this->getData();
		$result = [];
		foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
			// smartctl 5.41 2011-06-09 r3365
			// SMART Attributes Data Structure revision number: 16
			// Vendor Specific SMART Attributes with Thresholds:
			// ID# ATTRIBUTE_NAME          FLAGS    VALUE WORST THRESH FAIL RAW_VALUE
			//   1 Raw_Read_Error_Rate     POSR-K   100   100   051    -    48
			//   2 Throughput_Performance  -OS--K   055   055   000    -    18907
			//   3 Spin_Up_Time            PO---K   068   068   025    -    9773
			//   4 Start_Stop_Count        -O--CK   100   100   000    -    795
			//   5 Reallocated_Sector_Ct   PO--CK   252   252   010    -    0
			//   7 Seek_Error_Rate         -OSR-K   252   252   051    -    0
			//  12 Power_Cycle_Count       ------+  100   100   000    -    25
			// 187 Reported_Uncorrect      -O--CK   100   100   ---    -    0
			// 194 Temperature_Celsius     ------+  036   036   000    -    36 (Min/Max 19/45)
			$regex = '/^\s*(\d+)\s+(\S+)\s+([POSRCK-]+[+]?)\s+(\d+)\s+(\d+)\s+'.
				'(\d+|---)\s+(\S+)\s+(.+)$/i';
			if (1 == preg_match($regex, $cmdOutputv, $matches)) {
				$id = intval($matches[1]);
				$attrData = array(
					"id" => $id,
					"attrname" => $matches[2],
					"flags" => $matches[3],
					"value" => intval($matches[4]),
					"worst" => intval($matches[5]),
					"threshold" => ("---" == $matches[6]) ?
					  0 : intval($matches[6]),
					"whenfailed" => $matches[7],
					"rawvalue" => $matches[8],
					"description" => array_key_exists($id, $this->attrDesc) ?
					  $this->attrDesc[$id] : "",
					// Additional fields used for the assessment.
					"prefailure" => (FALSE !== stripos($matches[3], "P")),
					"assessment" => self::SMART_ASSESSMENT_BAD_STATUS
				);
				$this->assessAttribute($attrData);
				$result[] = $attrData;
			}
		}
		return $result;
	}

	private function assessAttribute(&$attrData) {
		if (TRUE === $attrData['prefailure']) {
			// Set the default assessment.
			$attrData['assessment'] = self::SMART_ASSESSMENT_GOOD;
			// Always-Fail and Always-Passing thresholds are not relevant
			// for our assessment.
			if (!(1 <= $attrData['threshold']) && (0xFD >= $attrData['threshold'])) {
				$attrData['assessment'] = self::SMART_ASSESSMENT_BAD_STATUS;
				return;
			}
			// See https://wiki.ubuntuusers.de/Festplattenstatus
			if ($attrData['value'] <= $attrData['threshold']) {
				$attrData['assessment'] = self::SMART_ASSESSMENT_BAD_ATTRIBUTE_NOW;
				return;
			}
			if ($attrData['worst'] <= $attrData['threshold']) {
				$attrData['assessment'] = self::SMART_ASSESSMENT_BAD_ATTRIBUTE_IN_THE_PAST;
				return;
			}
			// Check for bad sectors (Reallocated_Sector_Ct and Current_Pending_Sector).
			if ($attrData['id'] == 5 || $attrData['id'] == 197) {
				if (1 <= $attrData['rawvalue']) {
					$attrData['assessment'] = self::SMART_ASSESSMENT_BAD_SECTOR;
					return;
				}
			}
		} else {
			switch ($attrData['id']) {
			case 187: // Reported_Uncorrect
				// https://www.backblaze.com/blog/hard-drive-smart-stats/
				// Check number of reads that could not be corrected using
				// hardware ECC.
				if (0 < $attrData['rawvalue']) {
					$attrData['assessment'] = self::SMART_ASSESSMENT_BAD_ATTRIBUTE_NOW;
				}
				break;
			}
		}
	}

	/**
	 * Get a specific attribute by ID.
	 * @param int $id The attribute ID.
	 * @param any $default An optional default value. Defaults to FALSE.
	 * @return object|any An object with the requested attribute data,
	 *   otherwise the specified default value.
	 */
	public function getAttribute($id, $default = FALSE) {
		if (FALSE === ($attributes = $this->getAttributes()))
			return $default;
		$result = $default;
		foreach ($attributes as $attrk => $attrv) {
			if ($attrv['id'] == $id) {
				$result = $attrv;
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the S.M.A.R.T. Self-test log structure.
	 * @return array An array of S.M.A.R.T. self-test logs.
	 */
	public function getSelfTestLogs(): array {
		$this->getData();
		$result = [];
		foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
			// SMART Self-test log structure
			// Parse command output:
			// Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error
			// # 1  Extended offline    Completed: read failure       90%       670         57217755
			// # 2  Short captive       Interrupted (host reset)      80%      1392         -
			$regex = '/^#\s*(\d+)\s+(Short offline|Extended offline|'.
			  'Short captive|Extended captive)\s+(.+)\s+(\d+)%\s+(\d+)'.
			  '\s+(.+)$/';
			if (1 == preg_match($regex, $cmdOutputv, $matches)) {
				$result[] = array(
					"num" => $matches[1],
					"description" => $matches[2],
					"status" => $matches[3],
					"remaining" => $matches[4],
					"lifetime" => $matches[5],
					"lbaoffirsterror" => $matches[6]
				);
			}
		}
		return $result;
	}

	/**
	 * Get various device information.
	 * @return array An array of strings.
	 */
	public function getInformation(): array {
		$this->getData();
		// Initialize with default values. Note, the result list may
		// contain additional key/value pairs.
		$result = [
			"devicefile" => $this->sd->getDeviceFile(),
			"devicemodel" => "",
			"serialnumber" => "",
			"wwn" => $this->sd->getWorldWideName(),
			"firmwareversion" => "",
			"usercapacity" => ""
		];
		// INFORMATION SECTION
		// Parse command output:
		// === START OF INFORMATION SECTION ===
		// Model Family:     Western Digital RE3 Serial ATA family
		// Device Model:     WDC WD2502ABYS-02B7A0
		// Serial Number:    WD-WCAV1B245569
		// Firmware Version: 02.03B03
		// User Capacity:    251,059,544,064 bytes
		// Device is:        In smartctl database [for details use: -P show]
		// ATA Version is:   8
		// ATA Standard is:  Exact ATA specification draft version not indicated
		// Local Time is:    Tue Mar 11 10:18:42 2014 CET
		// SMART support is: Available - device has SMART capability.
		// SMART support is: Enabled
		// Power mode is:    ACTIVE
		//
		// === START OF READ SMART DATA SECTION ===
		// ...
		$sectionFound = FALSE;
		foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
			$cmdOutputv = trim($cmdOutputv);
			// Abort parsing, we are not interested in the information
			// shown below this line.
			if (in_array($cmdOutputv, [ "=== START OF READ SMART DATA SECTION ===",
					"=== START OF SMART DATA SECTION ===" ])) {
				break;
			}
			// Ignore everything that is not below this line.
			if ($cmdOutputv == "=== START OF INFORMATION SECTION ===") {
				$sectionFound = TRUE;
				continue;
			}
			// Have we found the information section?
			if (FALSE === $sectionFound) {
				continue;
			}
			// Parse the information section line:
			// Device Model:     WDC WD2502ABYS-02B7A0
			$regex = '/^([^:]+):\s+(.+)$/i';
			if (1 !== preg_match($regex, $cmdOutputv, $matches)) {
				continue;
			}
			// Convert the attribute name, e.g. 'Device Model' to
			// 'devicemodel'.
			$attrKey = mb_strtolower(str_replace(" ", "", $matches[1]));
			// Append key/value to result array.
			$result[$attrKey] = $matches[2];
		}
		// Map various keys if the user has installed an unsupported
		// smartmontools version.
		//
		// smartctl 7.3 2022-02-28 r5338 ...
		// ...
		// === START OF INFORMATION SECTION ===
		// Model Number:     WDC WD2502ABYS-02B7A0
		// Serial Number:    WD-WCAV1B245569
		// Firmware Version: 02.03B03
		// ...
		// === START OF SMART DATA SECTION ===
		$attrKeyMap = [
			'modelnumber' => 'devicemodel'
		];
		foreach ($attrKeyMap as $attrKeyk => $attrKeyv) {
			// Copy new attributes only if the origin attribute is empty.
			if (array_key_exists($attrKeyk, $result) &&
				array_key_exists($attrKeyv, $result) &&
				empty($result[$attrKeyv])
			) {
				$result[$attrKeyv] = $result[$attrKeyk];
			}
		}
		return $result;
	}

	/**
	 * Get the device temperature in °C (value only, no unit).
	 * @return string|boolean The temperature value, otherwise FALSE.
	 */
	public function getTemperature() {
		if (FALSE === ($attributes = $this->getAttributes()))
			return FALSE;
		$result = 0;
		$found = FALSE;
		// Process the attributes to get the temperature value.
		foreach ($attributes as $attrk => $attrv) {
			switch ($attrv['id']) {
			case 190:
				// ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
				// 190 Airflow_Temperature_Cel 0x0022   040   039   045    Old_age   Always   FAILING_NOW 60 (0 209 61 41)
				//
				// ID# ATTRIBUTE_NAME          FLAGS    VALUE WORST THRESH FAIL RAW_VALUE
				// 190 Airflow_Temperature_Cel -O---K   065   044   045    Past 35 (0 3 35 35 0)
			case 194:
				// ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
				// 194 Temperature_Celsius     0x0002   214   214   000    Old_age   Always       -       28 (Lifetime Min/Max 21/32)
				// 194 Temperature_Celsius     0x0022   060   061   000    Old_age   Always       -       60 (0 20 0 0)
				// 194 Temperature_Celsius     0x0022   030   055   000    Old_age   Always       -       30 (Min/Max 17/55)
				//
				// ID# ATTRIBUTE_NAME          FLAGS    VALUE WORST THRESH FAIL RAW_VALUE
				// 194 Temperature_Celsius     -O---K   076   037   ---    -    24 (Min/Max 19/37)
			case 231: // temperature-celsius
				// ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
				// 231 Temperature_Celsius     0x0013   100   100   010    Pre-fail  Always       -       0
				$regex = '/^(\d+)(\s+(.+))*$/';
				if (1 !== preg_match($regex, $attrv['rawvalue'], $matches))
					break;
				// Verify temperature.
				$temp = intval($matches[1]);
				if ((-15 > $temp) || (100 < $temp))
					break;
				if (!$found || ($temp > $result))
					$result = $temp;
				$found = TRUE;
				break;
			}
			if (TRUE === $found)
				break;
		}
		// If the SMART attributes are not present then it may be a SAS
		// or older SCSI device. Then the command output looks like:
		//
		// Device: SEAGATE  ST336605LSUN36G  Version: 0238
		// Serial number: 3FP1J35V00007241EEC7
		// Device type: disk
		// Local Time is: Thu Apr  5 15:41:56 2012 CEST
		// Device supports SMART and is Enabled
		// Temperature Warning Enabled
		// SMART Health Status: OK
		//
		// Current Drive Temperature:     34 C
		// Drive Trip Temperature:        65 C
		// ...
		//
		// Alternative output:
		// ...
		// SCT Status Version:                  3
		// SCT Version (vendor specific):       256 (0x0100)
		// SCT Support Level:                   1
		// Device State:                        DST executing in background (3)
		// Current Temperature:                    28 Celsius
		// Power Cycle Min/Max Temperature:     23/28 Celsius
		// Lifetime    Min/Max Temperature:     21/32 Celsius
		// ...
		//
		// Alternative output:
		// Warning  Comp. Temp. Threshold:     73 Celsius
		// Critical Comp. Temp. Threshold:     76 Celsius
		// Temperature:                        30 Celsius
		// Warning  Comp. Temperature Time:    0
		// Critical Comp. Temperature Time:    0
		// Temperature Sensor 1:               30 Celsius
		// Temperature Sensor 2:               35 Celsius
		// ...
		if (FALSE === $found) {
			$regex = '/^(Current Drive Temperature|Current Temperature|Temperature):'.
				'\s+(\d+)\s+(C|Celsius)$/i';
			foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
				if (1 == preg_match($regex, $cmdOutputv, $matches)) {
					$result = $matches[2];
					$found = TRUE;
					break;
				}
			}
		}
		return (TRUE === $found) ? $result : FALSE;
	}

	/**
	 * Get the count of power on/off cycles.
	 * @return int Returns the count of power on/off cycles, otherwise -1.
	 */
	public function getPowerCycleCount() {
		// Try to get the `Power_Cycle_Count` attribute.
		if (FALSE !== ($attrData = $this->getAttribute(12))) {
			return $attrData['rawvalue'];
		}
		// If the SMART attributes are not present then it may be a
		// NVMe device. In that case the command output looks like:
		//
		// === START OF SMART DATA SECTION ===
		// ...
		// Critical Warning:                   0x00
		// Temperature:                        39 Celsius
		// Available Spare:                    100%
		// ...
		// Power Cycles:                       196
		// Power On Hours:                     616
		// ...
		$info = $this->getInformation();
		return array_value($info, "powercycles", -1);
	}

	/**
	 * Get the count of hours in power-on state.
	 * @return int Returns the count of power on/off cycles, otherwise -1.
	 */
	public function getPowerOnHours() {
		// Try to get the `Power_On_Hours` attribute.
		if (FALSE !== ($attrData = $this->getAttribute(9))) {
			return $attrData['rawvalue'];
		}
		// If the SMART attributes are not present then it may be a
		// NVMe device. In that case the command output looks like:
		//
		// === START OF SMART DATA SECTION ===
		// ...
		// Critical Warning:                   0x00
		// Temperature:                        39 Celsius
		// Available Spare:                    100%
		// ...
		// Power Cycles:                       196
		// Power On Hours:                     616
		// ...
		$info = $this->getInformation();
		return array_value($info, "poweronhours", -1);
	}

	/**
	 * Get the overall assessment for the device.
	 * @return string Returns the following strings:
	 *    <ul>
	 *   \li GOOD
	 *   \li BAD_ATTRIBUTE_NOW
	 *   \li BAD_ATTRIBUTE_IN_THE_PAST
	 *   \li BAD_SECTOR
	 *   \li BAD_SECTOR_MANY
	 *   \li BAD_STATUS
	 *   </ul>
	 *   or otherwise FALSE.
	 */
	public function getOverallStatus() {
		if (FALSE === ($attributes = $this->getAttributes()))
			return FALSE;
		// If the SMART attributes are not present, e.g. SAS devices,
		// then try to get the information from the 'SMART DATA SECTION'.
		if (empty($attributes)) {
			// === START OF READ SMART DATA SECTION ===
			// SMART Health Status: OK
			//
			// Current Drive Temperature:     31 C
			// ...
			//
			// Alternative output:
			// Temperature Warning Enabled
			// SMART Health Status: LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED [asc=5d, ascq=2]
			//
			// Current Drive Temperature:     39 C
			// ...
			//
			// smartctl 6.6 2017-11-05 r4594 [x86_64-linux-5.10.0-0.bpo.9-amd64] (local build)
			// ...
			// === START OF READ SMART DATA SECTION ===
			// SMART overall-health self-assessment test result: PASSED
			// ...
			$regex = '/^SMART (Health Status:\s+OK|overall-health self-assessment test result:\s+PASSED)$/i';
			foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
				if (1 == preg_match($regex, $cmdOutputv)) {
					return self::SMART_ASSESSMENT_GOOD;
				}
			}
			return self::SMART_ASSESSMENT_BAD_STATUS;
		}
		// Checks are adapted from libatasmart.
		// See http://git.0pointer.net/libatasmart.git
		// Get number of bad sectors.
		$numSectors = 0;
		$attrData = $this->getAttribute(5); // Reallocated_Sector_Ct
		if (is_array($attrData))
			$numSectors += $attrData['rawvalue'];
		$attrData = $this->getAttribute(197); // Current_Pending_Sector
		if (is_array($attrData))
			$numSectors += $attrData['rawvalue'];
		// Check if the number of bad sectors is greater than a
		// certain threshold.
		// !!! Note, currently this check is only available on 64bit systems
		// because i'm too lazy to implement the log function using the BC
		// math library to support this check on 32bit systems (the getSize()
		// method may return really big integers for storage devices nowadays
		// which can't be handled on 32bit systems anymore).
		if (is_64bits() && !is_null($this->sd)) {
			// Get the size of the device in bytes.
			$size = intval($this->sd->getSize());
			$sectorThreshold = intval(log($size / 512) * 1024);
			if ($numSectors >= $sectorThreshold)
				return self::SMART_ASSESSMENT_BAD_SECTOR_MANY;
		}
		// Check if any of the SMART attributes is bad.
		foreach ($attributes as $attrk => $attrv) {
			// Skip attributes that do not have a valid assessment.
			if (self::SMART_ASSESSMENT_BAD_STATUS == $attrv['assessment'])
				continue;
			if (self::SMART_ASSESSMENT_GOOD !== $attrv['assessment'])
				return $attrv['assessment'];
		}
		// Check if there are any bad sectors at all.
		if ($numSectors > 0)
			return self::SMART_ASSESSMENT_BAD_SECTOR;
		// There's really nothing to complain about, so give it a pass.
		return self::SMART_ASSESSMENT_GOOD;
	}

	/**
	 * Get the power mode of the device.
	 * @return string Returns the power mode, e.g. `ACTIVE`, `STANDBY`,
	 *   `IDLE`, `ACTIVE or IDLE`, `UNKNOWN` or `ERROR`.
	 * @see https://www.smartmontools.org/browser/trunk/smartmontools/smartctl.8.in#n
	 */
	public function getPowerMode(): string {
		try {
			$info = $this->getInformation();
			// smartctl 7.3 2022-02-28 r5338 [x86_64-linux-6.1.0-0.deb11.21-amd64] (local build)
			// Copyright (C) 2002-22, Bruce Allen, Christian Franke, www.smartmontools.org
			//
			// === START OF INFORMATION SECTION ===
			// Model Family:     Western Digital Caviar Green (AF)
			// ...
			// SMART support is: Available - device has SMART capability.
			// SMART support is: Enabled
			// Power mode is:    ACTIVE or IDLE
			if (array_key_exists("powermodeis", $info)) {
				return $info['powermodeis'];
			}
		} catch (\Exception $e) {
			syslog(LOG_ERR, sprintf("Failed to fetch SMART information: %s",
				$e->getMessage()));
			return "ERROR";
		}
		// smartctl 7.3 2022-02-28 r5338 [x86_64-linux-6.1.0-0.deb11.21-amd64] (local build)
		// Copyright (C) 2002-22, Bruce Allen, Christian Franke, www.smartmontools.org
		//
		// Device is in STANDBY mode, exit(2)
		$regex = '/^Device is in (\S+) mode, exit\((\d+)\)$/i';
		if (1 == preg_match($regex, end($this->cmdOutput), $matches)) {
			return $matches[1];
		}
		return "UNKNOWN";
	}
}
