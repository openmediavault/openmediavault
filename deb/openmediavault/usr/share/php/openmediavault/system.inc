<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
require_once("openmediavault/globals.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");

/**
 * A class that implements a mount point.
 * @ingroup api
 */
class OMVMntEnt extends OMVObject {
	private $device = "";
	private $dir = "";

	/**
	 * Constructor
	 * @param device The block special device or remote file system,
	 *   e.g. /dev/sde1, /dev/disk/by-uuid/2218-DF1C or rootfs.
	 * @param dir The mount point for the file system, e.g. /media/2218-DF1C.
	 */
	public function __construct($device, $dir) {
		$this->device = $device;
		$this->dir = $dir;
	}

	/**
	 * Get the device.
	 * @return The device, otherwise FALSE.
	 */
	public function getDevice() {
		return empty($this->device) ? FALSE : $this->device;
	}

	/**
	 * Get the mount point.
	 * @return The mount point of the filesystem or FALSE.
	 */
	public function getMountPoint() {
		return empty($this->dir) ? FALSE : $this->dir;
	}

	/**
	 * Mount the given file system.
	 * @param options Additional mount options. Defaults to "".
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function mount($options = "") {
		$cmdArgs = array("-v");
		if (!empty($options)) {
			if (!is_array($options))
				$options = array($options);
			$cmdArgs[] = sprintf("-o %s", implode(",", $options));
		}
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = sprintf("export LANG=C; mount %s 2>&1", implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Unmount the given file system.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function umount($force = FALSE, $lazy = FALSE) {
		$cmdArgs = array("-v");
		if (TRUE === $force)
			$cmdArgs[] = "-f";
		if (TRUE === $lazy)
			$cmdArgs[] = "-l";
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = sprintf("export LANG=C; umount %s 2>&1", implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Create the mount point of the file system.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function mkDir() {
		if (is_dir($this->dir))
			return TRUE;
		return mkdir($this->dir, 0700, TRUE);
	}

	/**
	 * Unlink the mount point of the file system.
	 * @param force Set to TRUE to ignore nonexistent files. Defaults to TRUE.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function unlinkDir($force = TRUE) {
		$cmd = sprintf("export LANG=C; rm -rf %s 2>&1", escapeshellarg(
		  $this->dir));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Check if the given file system is mounted.
	 * @return TRUE if the file system is mounted, otherwise FALSE.
	 */
	public function isMounted() {
		$result = FALSE;
		foreach (file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if (($matches[0] == $this->device) || ($matches[1] == $this->dir)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * See if the directory is a mount point.
	 * @return TRUE if the directory is a mount point, FALSE if not.
	 */
	public function isMountPoint() {
		$cmd = sprintf("export LANG=C; mountpoint %s 2>&1", escapeshellarg(
		  $this->dir));
		@OMVUtil::exec($cmd, $output, $result);
		return ($result == 0) ? TRUE : FALSE;
	}

	/**
	 * Get the \OMV\System\Filesystem\Filesystem object for the given mount point.
	 * @param dir The file system path prefix.
	 * @return The class object or NULL on failure.
	 */
	public static function getFilesystemByDir($dir) {
		$result = NULL;
		foreach (file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if ($matches[1] == $dir) {
				$result = \OMV\System\Filesystem\Filesystem::getImpl($matches[0]);
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the \OMV\System\Filesystem\Filesystem object for the given mount point.
	 * @param name The name of mounted file system.
	 * @return The class object or NULL on failure.
	 */
	public static function getFilesystemByName($name) {
		$result = NULL;
		foreach (file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if ($matches[0] == $name) {
				$result = \OMV\System\Filesystem\Filesystem::getImpl($matches[0]);
				break;
			}
		}
		return $result;
	}
}

/**
 * Class implementing generic block devices.
 * @ingroup api
 */
interface OMVIBlockDevice {
	/**
	 * Checks if the device exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	function exists();

	/**
	 * Get the device path, e.g. /dev/sda.
	 * @return The device path.
	 */
	function getDeviceFile();

	/**
	 * Get the device name, e.g. sda or hdb.
	 * @param canonical If set to TRUE the canonical device file will
	 *   be used. Defaults to FALSE.
	 * @return The device name.
	 */
	function getDeviceName($canonical = FALSE);

	/**
	 * Get the device number, e.g. 8:17.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The device number as string or FALSE on failure.
	 */
	function getDeviceNumber();

	/**
	 * Get the major device number.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The major device number or FALSE on failure.
	 */
	function getMajor();

	/**
	 * Get the minor device number.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The minor device number or FALSE on failure.
	 */
	function getMinor();

	/**
	 * Get the size of the device in bytes.
	 * @return The size of the device in bytes, otherwise FALSE.
	 */
	function getSize();

	/**
	 * Get the blocksize of the device in bytes.
	 * @return The blocksize of the device in bytes, otherwise FALSE.
	 */
	function getBlockSize();

	/**
	 * Get the sectorsize of the device in bytes.
	 * @return The sectorsize of the device in bytes, otherwise FALSE.
	 */
	function getSectorSize();

	/**
	 * Get the description of the device.
	 * @return The device description.
	 */
	function getDescription();
}

/**
 * Abstract class implementing generic block devices.
 * @ingroup api
 */
abstract class OMVBlockDeviceAbstract extends OMVObject implements
  OMVIBlockDevice {
	protected $deviceFile = FALSE;
	protected $size = FALSE;
	protected $blockSize = FALSE;
	protected $sectorSize = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file.
	 */
	public function __construct($deviceFile) {
		$this->deviceFile = $deviceFile;
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		return is_block($this->getDeviceFile());
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFile() {
		return $this->deviceFile;
	}

	/**
	 * Get the canonical device file, e.g. /dev/mapper/vg0-lv0 -> /dev/dm-0
	 */
	public function getCanonicalDeviceFile() {
		return realpath($this->deviceFile);
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceName($canonical = FALSE) {
		// Get the device file and extract the name, e.g. /dev/sda => sda.
		$deviceName = str_replace("/dev/", "", !$canonical ?
		  $this->getDeviceFile() : $this->getCanonicalDeviceFile());
		return $deviceName;
	}

	/**
	 * See interface definition.
	 */
	public function getSize() {
		return $this->size;
	}

	/**
	 * See interface definition.
	 */
	public function getBlockSize() {
		return $this->blockSize;
	}

	/**
	 * See interface definition.
	 */
	public function getSectorSize() {
		return $this->sectorSize;
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceNumber() {
		// Get the canonical device name, e.g.
		// /dev/root => /dev/sda1
		$deviceName = $this->getDeviceName(TRUE);
		// Get the device number via sysfs.
		$filename = sprintf("/sys/class/block/%s/dev", $deviceName);
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * See interface definition.
	 */
	public function getMajor() {
		if (FALSE === ($devNum = $this->getDeviceNumber()))
			return FALSE;
		$devNumParts = explode(":", $devNum);
		return intval($devNumParts[0]);
	}

	/**
	 * See interface definition.
	 */
	public function getMinor() {
		if (FALSE === ($devNum = $this->getDeviceNumber()))
			return FALSE;
		$devNumParts = explode(":", $devNum);
		return intval($devNumParts[1]);
	}
}

/**
 * This class implements an interface to a generic Linux block device.
 * @ingroup api
 */
class OMVBlockDevice extends OMVBlockDeviceAbstract {
	/**
	 * See interface definition.
	 */
	public function getDescription() {
		return sprintf("Block device %s [%s]", $this->getDeviceName(),
		  $this->getDeviceNumber());
	}
}

/**
 * Abstract class implementing generic storage devices.
 * @ingroup api
 */
abstract class OMVStorageDeviceAbstract extends OMVBlockDeviceAbstract {
	protected $deviceFileById = FALSE;
	protected $model = FALSE;
	protected $vendor = FALSE;
	protected $udevInfo = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g.
	 * \li /dev/sdb
	 * \li /dev/md1
	 * \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 * \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	 */
	public function __construct($deviceFile) {
		if (preg_match("/^\/dev\/disk\/by-id\/.+$/", $deviceFile)) {
			$this->deviceFileById = $deviceFile;
		} else {
			$this->deviceFile = $deviceFile;
		}
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		return is_block(!$this->deviceFile ? $this->deviceFileById :
		  $this->deviceFile);
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFile() {
		if (!$this->deviceFile)
			$this->deviceFile = realpath($this->deviceFileById);
		return $this->deviceFile;
	}

	/**
	 * Get the device path by ID, e.g.
	 * \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	 * \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	 * \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
	 * The following order of paths will be retured if available:
	 * \li ata-xxx
	 * \li wwn-xxx
	 * \li scsi-xxx
	 * \li ...
	 * @return The device path (/dev/disk/by-id/xxx) if available,
	 *   otherwise /dev/xxx will be returned. In case of an error FALSE
	 *   will be returned.
	 */
	public function getDeviceFileById() {
		if (!$this->deviceFileById) {
			if (FALSE === ($devLinks = $this->getDeviceFileSymlinks()))
				return FALSE;
			// Set the device file by default.
			$this->deviceFileById = $this->getDeviceFile();
			// Get the 'by-id' device file symlinks.
			$deviceNamesById = array();
			$regex = "/^\/dev\/disk\/by-id\/(.+)$/";
			foreach ($devLinks as $devLinkk => $devLinkv) {
				if (1 == preg_match($regex, $devLinkv, $matches))
					$deviceNamesById[] = $matches[1];
			}
			if (!empty($deviceNamesById)) {
				// Sort the device names in the following order:
				// 1. ata-xxx
				// 2. wwn-xxx
				// 3. scsi-xxx
				// 4. ...
				//
				// Legend:
				// a=[a]ta-xxx
				// w=[w]wn-xxx
				// s=[s]csi-xxx
				// x=any other device file name
				// fn=call strnatcmp function
				//
				// Compare function matrix:
				// |      | $b=a | $b=w | $b=s | $b=x |
				// |------|------|------|------|------|
				// | $a=a |  fn  |  -1  |  -1  |  -1  |
				// | $a=w |   1  |  fn  |  -1  |  -1  |
				// | $a=s |   1  |   1  |  fn  |  -1  |
				// | $a=x |   1  |   1  |   1  |  fn  |
				//
				// Example:
				// ata-WDC_WD2002FAEX-007BA0_WD-WCAY01065572
				// wwn-0x50014ee25d4cdecd
				// scsi-SATA_WDC_WD2002FAEX-_WD-WCAY01065572
				// xxx...
				// yyy...
				usort($deviceNamesById, create_function('$a, $b',
				  '$map = array("a" => 0, "w" => 1, "s" => 2);'.
				  '$indexA = array_key_exists($a[0], $map) ? $map[$a[0]] : 3;'.
				  '$indexB = array_key_exists($b[0], $map) ? $map[$b[0]] : 3;'.
				  '$cmpMatrix = array('.
				  '    0 => array(0, -1, -1, -1),'.
				  '    1 => array(1,  0, -1, -1),'.
				  '    2 => array(1,  1,  0, -1),'.
				  '    3 => array(1,  1,  1,  0)'.
				  ');'.
				  '$cmpResult = $cmpMatrix[$indexA][$indexB];'.
				  'return (0 !== $cmpResult) ? $cmpResult : strnatcmp($a, $b);'));
				// Finally build the whole path of the device file.
				$this->deviceFileById = sprintf("/dev/disk/by-id/%s",
				  array_shift($deviceNamesById));
			}
		}
		return $this->deviceFileById;
	}

	/**
	* Get all device file symlinks via UDEV, e.g.
	* \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	* \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	* \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
	* \li /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
	* @return The device file symlinks as an array of strings. In case
	*   of an error FALSE will be returned.
	*/
	public function getDeviceFileSymlinks() {
		$cmd = sprintf("export LANG=C; udevadm info --query=symlink ".
		  "--name=%s", escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		$result = explode(" ", $output[0]);
		foreach ($result as &$deviceFile) {
			$deviceFile = sprintf("/dev/%s", $deviceFile);
		}
		return $result;
	}

	/**
	 * Get the size of the device in bytes.
	 * @return The size (64bit) of the device in bytes as string.
	 */
	public function getSize() {
		if ($this->size === FALSE) {
			$cmd = sprintf("export LANG=C; blockdev --getsize64 %s",
			  escapeshellarg($this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				$this->setLastError($output);
				return FALSE;
			}
			$this->size = $output[0];
		}
		return $this->size;
	}

	/**
	 * Get the blocksize of the device in bytes.
	 * @return The blocksize of the device in bytes, otherwise FALSE.
	 */
	public function getBlockSize() {
		if ($this->blockSize === FALSE) {
			$cmd = sprintf("export LANG=C; blockdev --getbsz %s",
			  escapeshellarg($this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				$this->setLastError($output);
				return FALSE;
			}
			$this->blockSize = intval($output[0]);
		}
		return $this->blockSize;
	}

	/**
	 * Get the sectorsize of the device in bytes.
	 * @return The sectorsize of the device in bytes, otherwise FALSE.
	 */
	public function getSectorSize() {
		if ($this->sectorSize === FALSE) {
			$cmd = sprintf("export LANG=C; blockdev --getss %s",
			  escapeshellarg($this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				$this->setLastError($output);
				return FALSE;
			}
			$this->sectorSize = intval($output[0]);
		}
		return $this->sectorSize;
	}

	/**
	 * Get the device model.
	 * @return The device model, otherwise an empty string.
	 */
	public function getModel() {
		if ($this->model === FALSE) {
			$filename = sprintf("/sys/block/%s/device/model",
			  $this->getDeviceName());
			if (file_exists($filename)) {
				$this->model = trim(file_get_contents($filename));
			} else {
				$this->model = "";
			}
		}
		return $this->model;
	}

	/**
	 * Get the device vendor.
	 * @return The device vendor, otherwise an empty string.
	 */
	public function getVendor() {
		if ($this->vendor === FALSE) {
			$filename = sprintf("/sys/block/%s/device/vendor",
			  $this->getDeviceName());
			if (file_exists($filename)) {
				$this->vendor = trim(file_get_contents($filename));
			} else {
				$this->vendor = "";
			}
		}
		return $this->vendor;
	}

	/**
	 * Get the device serial number.
	 * @return The device serial number, otherwise an empty string.
	 */
	public function getSerialNumber() {
		if (FALSE === ($property = $this->getUdevProperty("ID_SERIAL_SHORT")))
			return "";
		return str_replace("_", " ", $property);
	}

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		$model = $this->getModel();
		return sprintf("%s [%s, %s]", !empty($model) ? $model : gettext("n/a"),
		  $this->getDeviceFile(), binary_format($this->getSize()));
	}

	/**
	 * Check if the device is of rotational type or non-rotational type.
	 * See https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt
	 * @return TRUE if device is of rotational type, otherwise FALSE.
	 */
	public function isRotational() {
		// Use udev property.
		if (FALSE !== ($property = $this->getUdevProperty(
		  "ID_SSD"))) {
			if (0 == strcasecmp("1", $property))
				return FALSE;
		}
		if (FALSE !== ($property = $this->getUdevProperty(
		  "ID_ATA_ROTATION_RATE_RPM"))) {
			if (0 == strcasecmp("0", $property))
				return FALSE;
		}
		// Use kernel attribute.
		$filename = sprintf("/sys/block/%s/queue/rotational",
		  $this->getDeviceName());
		if (file_exists($filename)) {
			if ("0" == trim(file_get_contents($filename)))
				return FALSE;
		}
		// Use heuristic.
		$model = $this->getModel();
		if (is_string($model) && (FALSE !== strstr($model, "SSD")))
			return FALSE;
		return TRUE;
	}

	/**
	* Check if the device is removable.
	* @return TRUE if device is removable, otherwise FALSE.
	*/
	public function isRemovable() {
		$filename = sprintf("/sys/block/%s/removable",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return (trim(file_get_contents($filename)) == "1") ? TRUE : FALSE;
	}

	/**
	 * Check if the given device is a hardware/software RAID device.
	 * @return TRUE if the device is a hardware/software RAID, otherwise FALSE.
	 */
	public function isRaid() {
		return FALSE;
	}

	/**
	 * Check if the given device is an USB device.
	 * @return TRUE if the device is a connected via USB, otherwise FALSE.
	 */
	public function isUsb() {
		// Identify USB devices via 'ID_BUS=usb'.
		if (FALSE !== ($property = $this->getUdevProperty("ID_BUS"))) {
			if (0 == strcasecmp("usb", $property))
				return TRUE;
		}
		// Identify USB devices via 'ID_PATH=xxx-usb-xxx'.
		// Example:
		// ID_PATH=pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0
		// ID_PATH=pci-0000:00:12.2-usb-0:3:1.0-scsi-0:0:0:0
		if (FALSE !== ($property = $this->getUdevProperty("ID_PATH"))) {
			if (1 == preg_match('/^.+-usb-.+$/i', $property))
				return TRUE;
		}
		return FALSE;
	}

	/**
	 * Check if the given device is connected via ATA.
	 * @return TRUE if the device is connected via ATA, otherwise FALSE.
	 */
	public function isAta() {
		if(FALSE === ($property = $this->getUdevProperty("ID_BUS")))
			return FALSE;
		return (0 == strcasecmp("ata", $property)) ? TRUE : FALSE;
	}

	/**
	 * Check if the given device is read-only.
	 * @return TRUE if the device is read-only, otherwise FALSE.
	 */
	public function isReadOnly() {
		return FALSE;
	}

	/**
	 * Queries the udev database for device information stored in the udev
	 * database.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return The requested property, otherwise FALSE.
	 */
	protected function getUdevProperty($id) {
		if ($this->udevInfo === FALSE) {
			$cmd = sprintf("export LANG=C; udevadm info --query=property ".
			  "--name=%s", escapeshellarg($this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				$this->setLastError($output);
				return FALSE;
			}
			// Parse output:
			// UDEV_LOG=3
			// DEVPATH=/devices/pci0000:00/0000:00:10.0/host2/target2:0:1/2:0:1:0/block/sdb
			// MAJOR=8
			// MINOR=16
			// DEVNAME=/dev/sdb
			// DEVTYPE=disk
			// SUBSYSTEM=block
			// ID_SCSI=1
			// ID_VENDOR=VMware_
			// ID_VENDOR_ENC=VMware\x2c\x20
			// ID_MODEL=VMware_Virtual_S
			// ID_MODEL_ENC=VMware\x20Virtual\x20S
			//
			// DEVLINKS=/dev/disk/by-id/ata-IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-id/scsi-SATA_IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
			// DEVNAME=/dev/sda
			// DEVPATH=/devices/pci0000:00/0000:00:02.5/host0/target0:0:0/0:0:0:0/block/sda
			// DEVTYPE=disk
			// ID_ATA=1
			// ID_ATA_FEATURE_SET_HPA=1
			// ID_ATA_FEATURE_SET_HPA_ENABLED=1
			// ID_ATA_FEATURE_SET_PM=1
			// ID_ATA_FEATURE_SET_PM_ENABLED=1
			// ID_ATA_FEATURE_SET_SECURITY=1
			// ID_ATA_FEATURE_SET_SECURITY_ENABLED=0
			// ID_ATA_FEATURE_SET_SECURITY_ERASE_UNIT_MIN=22
			// ID_ATA_FEATURE_SET_SMART=1
			// ID_ATA_FEATURE_SET_SMART_ENABLED=0
			// ID_ATA_WRITE_CACHE=1
			// ID_ATA_WRITE_CACHE_ENABLED=1
			// ID_BUS=ata
			// ID_MODEL=IBM-DTTA-351010
			// ID_MODEL_ENC=IBM-DTTA-351010\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20
			// ID_PART_TABLE_TYPE=dos
			// ID_PATH=pci-0000:00:02.5-scsi-0:0:0:0
			// ID_PATH_TAG=pci-0000_00_02_5-scsi-0_0_0_0
			// ID_REVISION=T56OA7A3
			// ID_SCSI_COMPAT=SATA_IBM-DTTA-341050_WF0WABH1579
			// ID_SERIAL=IBM-DTTA-341050_WF0WABH1579
			// ID_SERIAL_SHORT=WF0WFJH1486
			// ID_TYPE=disk
			// MAJOR=8
			// MINOR=0
			// SUBSYSTEM=block
			// UDEV_LOG=3
			// USEC_INITIALIZED=16872806
			foreach ($output as $row) {
				$data = explode("=", $row);
				if (empty($data))
					continue;
				$this->udevInfo[$data[0]] = $data[1];
			}
		}
		if (!array_key_exists($id, $this->udevInfo))
			return FALSE;
		return $this->udevInfo[$id];
	}

	/**
	 * Check if the given device has S.M.A.R.T. support.
	 * @return TRUE if the device supports S.M.A.R.T., otherwise FALSE.
	 */
	public function hasSMART() {
		return ($this instanceof OMVStorageDeviceSMARTAbstract) ? TRUE : FALSE;
	}
}

/**
 * Abstract class implementing generic storage devices that support the
 * S.M.A.R.T. AT Attachment (ATA) interface.
 * @ingroup api
 */
abstract class OMVStorageDeviceSMARTAbstract extends OMVStorageDeviceAbstract {
	protected $smartDeviceType = "";

	/**
	 * Set the device type required by the smartctl command.
	 * @return Returns the device type.
	 */
	public function setSMARTDeviceType($type) {
		$this->smartDeviceType = $type;
		return $this->smartDeviceType;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSMARTDeviceType() {
		return $this->smartDeviceType;
	}

	public function getSMARTInformation() {
		return new OMVATASMARTInformation($this->getDeviceFile(),
		  $this->getSMARTDeviceType(), $this);
	}
}

/**
 * The generic class that represents a storage device backend.
 * @ingroup api
 *
 * To implement a new storage backend the following information are
 * necessary:
 * # cat /proc/partitions
 * # export LANG=C; blkid
 * # export LANG=C; udevadm info --query=property --name=/dev/<DEVNAME>
 * # cat /sys/block/<DEVNAME>/device/model
 * # cat /sys/block/<DEVNAME>/device/vendor
 */
abstract class OMVStorageDeviceBackendAbstract extends OMVObject {
	/**
	 * Get the type of the storage device backend, e.g.
	 * OMV_STORAGE_DEVICE_TYPE_HDD, OMV_STORAGE_DEVICE_TYPE_HBA, ...
	 */
	abstract function getType();

	/**
	 * Get a list of devices of the given storage device backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	abstract function enumerate();

	/**
	 * Get a list of all devices that are used by devices of this
	 * storage device backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	public function enumerateSlaves() {
		return array();
	}

	/**
	 * Check whether the given device is represented by this storage
	 * device backend.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function isTypeOf($deviceFile) {
		return FALSE;
	}

	/**
	 * Get the object of the class that represents and implements a device
	 * of this storage device backend.
	 * @param args The arguments to the class constructor.
	 * @return The object of the class implementing the given device type,
	 *   otherwise NULL.
	 */
	public function getImpl($args) {
		return NULL;
	}

	/**
	 * Returns base device file by stripping the partition appendix.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb1 => /dev/sdb
	 *   \li /dev/cciss/c0d0p2 => /dev/cciss/c0d0
	 *   \li /dev/mapper/vg0-lv0 => /dev/mapper/vg0-lv0
	 *   \li /dev/dm-0 => /dev/dm-0
	 *   \li /dev/md0 => /dev/md0
	 *   \li /dev/loop0 => /dev/loop0
	 *   </ul>
	 * @return The base device file.
	 */
	public function baseDeviceFile($deviceFile) {
		return $deviceFile;
	}

	/**
	 * Return the device file to be used to create a filesystem.
	 * @note Only one partition per storage device is supported.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb => /dev/sdb1
	 *   \li /dev/cciss/c0d0 => /dev/cciss/c0d0p1
	 *   \li /dev/mapper/vg0-lv0 => /dev/mapper/vg0-lv0
	 *   \li /dev/dm-0 => /dev/dm-0
	 *   \li /dev/md0 => /dev/md0
	 *   </ul>
	 * @return The device file. Defaults to /dev/xxx1.
	 */
	public function fsDeviceFile($deviceFile) {
		return sprintf("%s1", $deviceFile);
	}

	/**
	 * Helper function to enumerate the devices represented by this storage
	 * device backend via the /proc filesystem.
	 * @param regex The regular expression used to identify the devices
	 *   represented by this storage backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	final protected function enumerateProcFs($regex) {
		$result = array();
		$regex = sprintf('/^\s+(\d+)\s+(\d+)\s+(\d+)\s+(%s)$/', $regex);
		foreach(file("/proc/partitions") as $outputk => $outputv) {
			if(1 !== preg_match($regex, $outputv, $matches))
				continue;
			$result[] = sprintf("/dev/%s", $matches[4]);
		}
		return $result;
	}

	/**
	 * Helper function to check whether the given device is represented by
	 * this storage device backend.
	 * @param deviceFile Specifies the device file.
	 * @param regex The regular expression used to identify the devices
	 *   represented by this storage backend.
	 */
	final protected function isTypeOfByName($deviceFile, $regex) {
		// If the device file looks like /dev/disk/by-(id|label|path|uuid)/*
		// then it is necessary to get the /dev/xxx equivalent.
		if(1 == preg_match("/^\/dev\/disk\/by-.+$/", $deviceFile))
			$deviceFile = realpath($deviceFile);
		// Extract the device name, e.g.:
		// - /dev/sdb => sdb
		// - /dev/cciss/c0d0 => cciss/c0d0
		// - /dev/mapper/vg0-lv0 => mapper/vg0-lv0
		// - /dev/loop1 => loop1
		$deviceName = str_replace("/dev/", "", $deviceFile);
		// Check if the device name matches the given regular expression.
		return (1 == preg_match("/^{$regex}$/i", $deviceName));
	}
}

/**
 * Implements the storage device backend for Linux loop devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendLoop extends OMVStorageDeviceBackendAbstract {
	protected $regex = "loop[0-9]+";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceLoop($args);
	}

	function fsDeviceFile($deviceFile) {
		return $deviceFile;
	}
}

/**
 * Implements the storage device backend for Linux Bcache devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendBcache extends OMVStorageDeviceBackendAbstract {
	protected $regex = "bcache[0-9]+";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if (FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			$slaves = $object->getSlaves();
			if (FALSE === $slaves)
				continue;
			$result = array_merge($result, $slaves);
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceBcache($args);
	}
}

/**
 * Implements the storage device backend for Linux device mapper devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendDM extends OMVStorageDeviceBackendAbstract {
  	protected $regex = "(mapper\/.+)|(dm-\d+)";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if (FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			$slaves = $object->getSlaves();
			if (FALSE === $slaves)
				continue;
			$result = array_merge($result, $slaves);
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceDM($args);
	}

	function fsDeviceFile($deviceFile) {
		return $deviceFile;
	}
}

/**
 * Implements the storage device backend for LVM2.
 * @ingroup api
 */
class OMVStorageDeviceBackendLVM extends OMVStorageDeviceBackendDM {
	function enumerate() {
		$devs = parent::enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			// Check if the given device is a LVM logical volume.
			$object = new OMVLvmLogicalVolume($devv);
			if(!$object->exists())
				continue;
			$result[] = $object->getDeviceFile();
		}
		return $result;
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			// Process LVM logical volumes.
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			// Get the physical volumes.
			$slaves = $object->getSlaves();
			if (FALSE !== $slaves)
				$result = array_merge($result, $slaves);
			// Get the volume group.
			$result[] = sprintf("/dev/%s", $object->getVGName());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		// Check if the given device is of type device mapper.
		if(FALSE === parent::isTypeOf($deviceFile))
			return FALSE;
		// Check if it is a LVM logical volume.
		$object = new OMVLvmLogicalVolume($deviceFile);
		return $object->exists();
	}

	function getImpl($args) {
		return new OMVLvmLogicalVolume($args);
	}
}

/**
 * Implements the storage device backend for Linux Mdadm software RAID
 * devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendMdadm extends OMVStorageDeviceBackendAbstract {
	protected $regex = "md\d+(p\d+)*";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if (FALSE === $devs)
			return FALSE;
		$result = array();
		foreach ($devs as $devk => $devv) {
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			$slaves = $object->getSlaves();
			if (FALSE === $slaves)
				continue;
			$result = array_merge($result, $slaves);
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceMdadm($args);
	}

	function fsDeviceFile($deviceFile) {
		return $deviceFile;
	}
}

/**
 * Implements the storage device backend for CCISS HBA devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendCCISS extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HBA;
	}

	function enumerate() {
		return $this->enumerateProcFs("cciss\/c[0-9]+d[0-9]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/cciss/c0d0
		// - /dev/cciss/c0d0p2
		$regex = "cciss\/c[0-9]+d[0-9]+(p[0-9]+)?";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceCCISS($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(p\d+)$/", "", $deviceFile);
	}

	function fsDeviceFile($deviceFile) {
		// E.g. /dev/cciss/c0d0p1
		return sprintf("%sp1", $deviceFile);
	}
}

/**
 * Implements the storage device backend for I2O based HBA devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendI2O extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HBA;
	}

	function enumerate() {
		return $this->enumerateProcFs("i2o\/hd[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/i2o/hda
		// - /dev/i2o/hdb1
		$regex = "i2o\/hd[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceI2O($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}

/**
 * Implements the storage device backend for harddisk devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendHDD extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		// Examples:
		// - /dev/hda
		// - /dev/sdb
		// - /dev/xvda
		return $this->enumerateProcFs("(h|s|xv)d[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/hdc2
		// - /dev/sdb
		// - /dev/sda1
		// - /dev/xvda
		// - /dev/xvdb2
		$regex = "(h|s|xv)d[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceHDD($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}

/**
 * Implements the storage device backend for CD-ROM devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendCDROM extends OMVStorageDeviceBackendAbstract {
	protected $regex = "sr[0-9]+";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/sr0
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceCDROM($args);
	}
}

/**
 * Implements the storage device backend for Multimedia Card (MMC) devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendMMC extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		// major minor  #blocks  name
		// 179        0   30948352 mmcblk0
		// 179        1   30947328 mmcblk0p1
		return $this->enumerateProcFs("mmcblk[0-9]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/mmcblk0
		// - /dev/mmcblk0p1
		$regex = "mmcblk[0-9]+(p[0-9]+)*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceMMC($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(p\d+)$/", "", $deviceFile);
	}

	function fsDeviceFile($deviceFile) {
		// E.g. /dev/mmcblk0p1
		return sprintf("%sp1", $deviceFile);
	}
}

/**
 * Implements the storage device backend for FusionIO devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendFIO extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		// major minor  #blocks  name
		//    8        0  312571224 sda
		//    8        1    1052226 sda1
		//    8        2    8393962 sda2
		//  252        0  157143584 fioa
		//  252        1  157139766 fioa1
		return $this->enumerateProcFs("fio[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/fioa
		// - /dev/fioa1
		$regex = "fio[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceHDD($args);
	}
}

/**
 * Implements the storage device backend for KVM VirtIO devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendVirtIO extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		// major minor  #blocks  name
		//  254        0  976762584 vda
		//  254       16  976762584 vdb
		//   11        0     355328 sr0
		//    8        0    8388608 sda
		//    8        1    7993344 sda1
		//    8        2          1 sda2
		//    8        5     392192 sda5
		return $this->enumerateProcFs("vd[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/vda
		// - /dev/vda1
		$regex = "vd[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceVirtIO($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}

/**
 * Implements the storage device backend for Non Volatile Memory (NVM) devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendNVM extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		// major minor  #blocks  name
		// 259        0  781412184 nvme0n1
		// 259        1  390705068 nvme0n1p1
		// 259        2  390706008 nvme0n1p2
		return $this->enumerateProcFs("nvme[0-9]+n[0-9]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/nvme<controller>n<namespace>p<partition>
		// - /dev/nvme0n1
		// - /dev/nvme0n1p1
		$regex = "nvme[0-9]+n[0-9]+(p[0-9]+)*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceNVM($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(p\d+)$/", "", $deviceFile);
	}

	function fsDeviceFile($deviceFile) {
		// E.g. /dev/nvme0n1p1
		return sprintf("%sp1", $deviceFile);
	}
}

/**
 * Implements the storage device backend for Ceph's RADOS Block Devices (RBD).
 * @ingroup api
 */
class OMVStorageDeviceBackendRDB extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		// major minor  #blocks  name
		// 251       32       1024 rbd2
		// 251       33          0 rbd2p1
		// 251       34          0 rbd2p2
		return $this->enumerateProcFs("rbd[0-9]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/rbd1
		// - /dev/rbd1p1
		$regex = "rbd[0-9]+(p[0-9]+)*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceRDB($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(p\d+)$/", "", $deviceFile);
	}

	function fsDeviceFile($deviceFile) {
		// E.g. /dev/rbd2p1
		return sprintf("%sp1", $deviceFile);
	}
}

/**
 * Helper class to get all existing devices of the given storage type.
 * @ingroup api
 */
class OMVStorageDevices extends OMVObject {
	static private $backends = array();

	/**
	 * Register a storage device backend. If the registered backend class is
	 * derived from another backend, then it will be inserted before this.
	 * @param backend The storage device backend object to register.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final static function registerBackend(OMVStorageDeviceBackendAbstract $backend) {
		if (!isset($backend))
			return FALSE;
		if (FALSE === ($parent = get_parent_class($backend))) {
			self::$backends[] = $backend;
		} else {
			// Try to find the derived class and the position where to
			// insert the storage device backend class to register.
			$offset = FALSE;
			foreach (self::$backends as $backendk => $backendv) {
				if ( ("OMVStorageDeviceBackendAbstract" === $parent) ||
				  (FALSE === ($backendv instanceof $parent)))
					continue;
				$offset = $backendk;
				break;
			}
			if (FALSE === $offset) {
				self::$backends[] = $backend;
			} else {
				array_splice(self::$backends, $offset, 0, array($backend));
			}
		}
		return TRUE;
	}

	/**
	 * Get the backend of the given device.
	 * @param deviceFile Specifies the device file.
	 * @return The backend that implements the given device file, otherwise
	 *   NULL.
	 */
	final static function getBackend($deviceFile) {
		$result = NULL;
		foreach (self::$backends as $backendk => $backendv) {
			if (FALSE === $backendv->isTypeOf($deviceFile))
				continue;
			$result = $backendv;
			break;
		}
		return $result;
	}

	/**
	 * Enumerate devices matching the given storage device type.
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	public static function enumerate($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerate()))
				return FALSE;
			$result = array_unique(array_merge($result, $devs));
		}
		return $result;
	}

	/**
	 * Enumerate all unused devices. This list contains all devices of the
	 * given type except the devices that are used by other storage devices
	 * (e.g. LVM as physical volume or a member of a Software RAID).
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	final static function enumerateUnused($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		// Append all available storage devices.
		if (FALSE === ($result = self::enumerate($type)))
			return FALSE;
		// Remove used devices.
		foreach (self::$backends as $backendk => $backendv) {
// Always collect and remove all slave devices.
//			if (!($type & $backendv->getType()))
//				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_diff($result, $slaves);
		}
		return $result;
	}

	/**
	 * Enumerate all used devices. The list contains all those devices that
	 * are used by the given storage devices, e.g. all members of a LVM
	 * or Software RAID.
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	final static function enumerateUsed($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_unique(array_merge($result, $slaves));
		}
		return $result;
	}

	/**
	 * Check if the given device is used/consumed by another storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return TRUE if the given device is used/consumed by another storage
	 *   device, otherwise FALSE.
	 */
	final static function isUsed($deviceFile, $type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = FALSE;
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (in_array($deviceFile, $slaves)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the object of the class which implements the given storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return The object of the class implementing the given storage device,
	 *   otherwise NULL.
	 */
	public static function getImpl($deviceFile) {
		if (NULL == ($backend = self::getBackend($deviceFile)))
			return NULL;
		return $backend->getImpl($deviceFile);
	}
}

/**
 * Factory to create objects of storage device classes.
 * @ingroup api
 */
class OMVStorageDeviceFactory extends OMVObject {
	/**
	 * Get the object of the class implementing the given storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return The object of the class implementing the given storage device,
	 *   otherwise NULL.
	 */
	public static function get($deviceFile) {
		return OMVStorageDevices::getImpl($deviceFile);
	}
}

/**
 * This class provides a simple interface to handle ATA/SATA and SCSI
 * hard disk devices. Try to use sysfs only to get the required information,
 * otherwise the disk will spin-up if it is in sleep mode.
 * @ingroup api
 */
class OMVStorageDeviceHDD extends OMVStorageDeviceSMARTAbstract {
	/**
	 * Get the current power mode status.
	 * @return The current power mode status which can be
	 *   <ul>
	 *   \li unknown
	 *   \li active/idle
	 *   \li standby
	 *   \li sleeping
	 *   </ul>
	 *   or FALSE on failure.
	 */
	public function getPowerModeState() {
		$result = FALSE;
		$cmd = sprintf("export LANG=C; hdparm -C %s 2>&1",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		// Parse command output:
		// /dev/sda:
		//  drive state is:  active/idle
		//
		// /dev/sdb:
		//  drive state is:  standby
		foreach ($output as $outputk => $outputv) {
			$regex = "/^\s+drive state is:\s+(\S+)$/i";
			if (1 == preg_match($regex, $outputv, $matches)) {
				$result = $matches[1];
				break;
			}
		}
		return $result;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSMARTDeviceType() {
		$type = parent::getSMARTDeviceType();
		if (empty($type)) {
			// TODO: The device type identification must be improved.
			if (TRUE === $this->isUsb())
				$type = $this->setSMARTDeviceType("sat");
		}
		return $type;
	}
}

/**
 * This class provides a simple interface to handle CD-ROM devices.
 * @ingroup api
 */
class OMVStorageDeviceCDROM extends OMVStorageDeviceAbstract {
	public function isReadOnly() {
		return TRUE;
	}
}

/**
 * This class provides a simple interface to handle Multimedia Card
 * (MMC) devices.
 * @ingroup api
 */
class OMVStorageDeviceMMC extends OMVStorageDeviceAbstract {
	public function isRotational() {
		return FALSE;
	}
}

/**
 * This class provides a simple interface to handle Non Volatile Memory
 * (NVM) devices.
 * @ingroup api
 */
class OMVStorageDeviceNVM extends OMVStorageDeviceAbstract {
	public function isRotational() {
		return FALSE;
	}
}

/**
 * This class provides a simple interface to handle Ceph's RADOS Block
 * Devices (RBD).
 * @ingroup api
 */
class OMVStorageDeviceRDB extends OMVStorageDeviceAbstract {
	public function isRotational() {
		// It is a virtual device.
		return FALSE;
	}
}

/**
 * This class provides a simple interface to handle the CCISS hardware RAID
 * devices, e.g. HP's Smart Array hardware RAID controller.
 * @ingroup api
 */
class OMVStorageDeviceCCISS extends OMVStorageDeviceSMARTAbstract {
	public function getDeviceName($canonical = FALSE) {
		// Get the device name and convert '/' character to '!', e.g.
		// cciss/c0d0 => cciss!c0d0.
		return strtr(parent::getDeviceName($canonical), "/", "!");
	}

	public function isRaid() {
		return TRUE;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSMARTDeviceType() {
		$type = parent::getSMARTDeviceType();
		// Note, 'auto' attempts to guess the device type from the device
		// name or from controller type info provided by the operating system
		// or from a matching USB ID entry in the drive database. Thus we
		// have to do the job ourself for various hardware RAID devices.
		if(empty($type) || ("auto" === $type)) {
			if(1 == preg_match("/^cciss!c(\d)d(\d)$/", $this->getDeviceName(),
			  $matches)) {
				$type = $this->setSMARTDeviceType(sprintf("cciss,%s",
				  $matches[2]));
			}
		}
		return $type;
	}
}

/**
 * This class provides a simple interface to handle the I2O based hardware
 * RAID devices.
 * @ingroup api
 */
class OMVStorageDeviceI2O extends OMVStorageDeviceAbstract {
	public function isRaid() {
		return TRUE;
	}
}

/**
 * This class provides a simple interface to handle software RAID devices.
 * @ingroup api
 */
class OMVStorageDeviceMdadm extends OMVStorageDeviceAbstract {
	protected $name = "";
	protected $level = FALSE;
	protected $devices = array();
	protected $numDevices = FALSE;
	protected $uuid = FALSE;
	protected $state = "unknown";
	protected $hasWriteIntentBitmap = FALSE;
	protected $details = array();
	private $dataCached = FALSE;

	/**
	 * Get the software RAID device detailed information.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if($this->dataCached !== FALSE)
			return TRUE;

		// ARRAY /dev/md0 level=raid5 num-devices=3 metadata=1.2 name=xxxx:0 UUID=a4266bf7:c671b343:c3d6e535:ca455e37
		//    devices=/dev/sdb,/dev/sdc,/dev/sdd
		$cmd = sprintf("export LANG=C; mdadm --detail --brief " .
		  "--verbose %s", escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}

		$attributes = array(
			"name" => "",
			"level" => "",
			"num-devices" => -1,
			"uuid" => "",
			"devices" => ""
		);
		$output = explode(" ", implode(" ", $output));
		foreach($output as $outputk => &$outputv) {
			$keyValue = explode("=", $outputv);
			if(count($keyValue) != 2)
				continue;
			$attributes[strtolower($keyValue[0])] = $keyValue[1];
		}

		$this->name = $attributes['name'];
		$this->level = $attributes['level'];
		$this->numDevices = intval($attributes['num-devices']);
		$this->uuid = $attributes['uuid'];
		$this->devices = explode(",", $attributes['devices']);

		// Sort the devices using a "natural order" algorithm.
		sort($this->devices, SORT_NATURAL);

		// Get more information.
		$cmd = sprintf("export LANG=C; mdadm --detail %s",
		  escapeshellarg($this->getDeviceFile()));
		unset($output);
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}

		// Store the whole command output.
		array_shift($output);
		$this->details = $output;

		// Parse command output:
		// /dev/md0:
		//         Version : 1.2
		//   Creation Time : Tue Dec 25 21:58:20 2012
		//      Raid Level : raid5
		//      Array Size : 207872 (203.03 MiB 212.86 MB)
		//   Used Dev Size : 103936 (101.52 MiB 106.43 MB)
		//    Raid Devices : 3
		//   Total Devices : 3
		//     Persistence : Superblock is persistent
		//
		//   Intent Bitmap : Internal
		//
		//     Update Time : Tue Dec 25 22:31:32 2012
		//           State : active
		//  Active Devices : 3
		// Working Devices : 3
		//  Failed Devices : 0
		//   Spare Devices : 0
		//
		//          Layout : left-symmetric
		//      Chunk Size : 512K
		//
		//            Name : dhcppc2:0  (local to host dhcppc2)
		//            UUID : 9d85a4f6:afff2cb6:b8a5f4dc:75f3cfd3
		//          Events : 37
		//
		//     Number   Major   Minor   RaidDevice State
		//        0       8       16        0      active sync   /dev/sdb
		//        1       8       48        1      active sync   /dev/sdd
		//        2       8       64        2      active sync   /dev/sde
		foreach($output as $outputk => $outputv) {
			if(empty($outputv))
				continue;
			$parts = explode(":", $outputv);
			if(empty($parts))
				continue;
			$parts = array_map("trim", $parts);
			switch(strtolower($parts[0])) {
			case "state":
				$this->state = $parts[1];
				break;
			case "intent bitmap":
				$this->hasWriteIntentBitmap = (0 == strcasecmp($parts[1],
				  "Internal")) ? TRUE : FALSE;
				break;
			}
		}

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		if($this->getData() === FALSE)
			return FALSE;
		return ($this->level !== FALSE);
	}

	/**
	 * Get the array name.
	 * @return The array name, otherwise FALSE.
	 */
	public function getName() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the level of the array.
	 * @return The level of the array, otherwise FALSE.
	 */
	public function getLevel() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->level;
	}

	/**
	 * Get the number of active devices in the array.
	 * @return The number of active devices in the array, otherwise FALSE.
	 */
	public function getNumDevices() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->numDevices;
	}

	/**
	 * Get the UUID of the array.
	 * @return The UUID of the array, otherwise FALSE.
	 */
	public function getUuid() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * See abstract class definition.
	 */
	public function getSerialNumber() {
		return $this->getUuid();
	}

	/**
	 * Get the list of device files used in the array.
	 * @return A list of device files used in the array, otherwise FALSE.
	 */
	public function getSlaves() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->devices;
	}

	public function getState() {
		if ($this->getData() === FALSE)
			return FALSE;
		// Parse command output:
		// Personalities : [linear] [multipath] [raid0] [raid1] [raid6] [raid5] [raid4] [raid10]
		// md1 : active raid5 sde[2] sdg[1] sdf[0]
		//       207872 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
		//       [=========>...........]  resync = 45.0% (47488/103936) finish=0.0min speed=47488K/sec
		//
		// md0 : active (auto-read-only) raid5 sdd[2] sdc[1] sdb[0]
		//       207872 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
		//       	resync=PENDING
		//
		// md0 : active raid5 sdf[4] sde[3] sdd[2] sdc[1] sdb[0]
		//       311808 blocks super 1.2 level 5, 512k chunk, algorithm 2 [5/5] [UUUUU]
		//       [====>................]  reshape = 20.5% (21504/103936) finish=0.0min speed=21504K/sec
		//
		// md0 : active raid1 sdc[2] sdd[0]
		//       2930135360 blocks super 1.2 [2/1] [U_]
		//       [>....................]  recovery = 1.1% (33131904/2930135360) finish=266.5min speed=181134K/sec
		//
		// md0 : inactive sdb[0](S) sdc[2](S) sdd[1](S)
		//       311808 blocks super 1.2
		//
		// md0 : active raid0 sdd[0] sde[1]
		//       311296 blocks super 1.2 512k chunks
		//
		// unused devices: <none>
		$regex = sprintf('/^(%s\s*:.*?)^\s*$/ims', $this->getDeviceName());
		if(1 !== preg_match($regex, file_get_contents("/proc/mdstat"),
		  $matches))
			return FALSE;
		$mdstat = $matches[1];
		// Extract the resync|reshape|recovery progress state.
		$progress = "unknown";
		if (1 == preg_match("/^.+(resync|reshape|recovery)\s*=\s*(.+)$/im",
		  $mdstat, $matches))
			$progress = trim($matches[2]);
		$state = $this->state;
		if ((FALSE !== stristr($state, "resyncing")) ||
		  (FALSE !== stristr($state, "recovering"))) {
			$state = sprintf("%s (%s)", $state, $progress);
		} else if (FALSE !== stristr($mdstat, "resync")) {
			$state = sprintf("%s, resyncing (%s)", $state, $progress);
		}
		return $state;
	}

	/**
	 * Get detail of the md device.
	 * @return The detail of the md device, otherwise FALSE.
	 */
	public function getDetail() {
		if($this->getData() === FALSE)
			return FALSE;
		return implode("\n", $this->details);
	}

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		return sprintf("Software RAID %s [%s, %s, %s]", $this->getName(),
		  $this->getDeviceFile(), $this->getLevel(),
		  binary_format($this->getSize()));
	}

	/**
	 * See abstract class definition.
	 */
	public function isRaid() {
		return TRUE;
	}

	/**
	 * Check whether the RAID device has write-intent bitmap enabled.
	 * Note, only 'internal' bitmaps are detected and reported.
	 * @see https://raid.wiki.kernel.org/index.php/Write-intent_bitmap
	 * @return TRUE if write-intent bitmap is enabled, otherwise FALSE.
	 */
	public function hasWriteIntentBitmap() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->hasWriteIntentBitmap;
	}
}

/**
 * This class provides a simple interface to handle device-mapper devices.
 * See http://en.wikipedia.org/wiki/Device_mapper for more details.
 * @ingroup api
 */
class OMVStorageDeviceDM extends OMVStorageDeviceAbstract {
	/**
	 * Constructor
	 * @param deviceFile Specifies the device file, e.g. /dev/dm-1,
	 *   /dev/vg0/lv0 or /dev/mapper/vg0-lv0.
	 */
	public function __construct($deviceFile) {
		// Call parent constructor.
		parent::__construct($deviceFile);
		// Any devices of the form /dev/dm-n are for internal use only and
		// should never be used. Because of that the device file returned
		// should look like /dev/mapper/<xyz>. See for more information:
		// https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html-single/DM_Multipath
		if(1 == preg_match("/^\/dev\/dm-\d+$/", $this->deviceFile)) {
			if(FALSE !== ($name = $this->getDeviceMapperName()))
				$this->deviceFile = sprintf("/dev/mapper/%s", $name);
		}
	}

	/**
	 * Get the name of the device mapper device.
	 * @return The name of the device mapper device.
	 */
	public function getDeviceMapperName() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/dm/name", $this->getDeviceName(TRUE));
		if(!file_exists($path))
			return FALSE;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the UUID of the device mapper device.
	 * @return The UUID of the device mapper device, otherwise FALSE.
	 */
	public function getDeviceMapperUuid() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/dm/uuid", $this->getDeviceName(TRUE));
		if(!file_exists($path))
			return FALSE;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the slave devices of the device mapper device.
	 * @return An array of device files, otherwise FALSE.
	 */
	public function getSlaves() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/slaves", $this->getDeviceName(TRUE));
		if(!file_exists($path))
			return FALSE;
		$result = array();
		$dir = new DirectoryIterator($path);
		foreach ($dir as $item) {
			if ($item->isDot() || !$item->isLink())
				continue;
			$result[] = sprintf("/dev/%s", $item->getFilename());
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Get the description of the device mapper device.
	 * @return The logical volume description, FALSE on failure.
	 */
	public function getDescription() {
		return sprintf(gettext("Device Mapper %s [%s, %s]"),
		  $this->getDeviceMapperName(), $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}
}

/**
 * This class provides a simple interface to handle loop devices.
 * @ingroup api
 */
class OMVStorageDeviceLoop extends OMVStorageDeviceAbstract {
	/**
	 * Get the description of the device.
	 * @return The device description, FALSE on failure.
	 */
	public function getDescription() {
		return sprintf(gettext("Loop device [%s, %s]"),
		  $this->getDeviceFile(), binary_format($this->getSize()));
	}
}

/**
 * This class provides a simple interface to handle Linux Bcache devices.
 * @ingroup api
 */
class OMVStorageDeviceBcache extends OMVStorageDeviceAbstract {
	/**
	 * Get the slave devices of the Bcache device.
	 * @return An array of device files, otherwise FALSE.
	 */
	public function getSlaves() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/slaves", $this->getDeviceName(TRUE));
		if (!file_exists($path))
			return FALSE;
		$result = array();
		$dir = new DirectoryIterator($path);
		foreach ($dir as $item) {
			if ($item->isDot() || !$item->isLink())
				continue;
			$result[] = sprintf("/dev/%s", $item->getFilename());
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Get the description of the device.
	 * @return The device description, FALSE on failure.
	 */
	public function getDescription() {
		return sprintf(gettext("Block layer cache [%s, %s]"),
		  $this->getDeviceFile(), binary_format($this->getSize()));
	}
}

/**
 * This class provides a simple interface to handle KVM VirtIO devices.
 * @ingroup api
 */
class OMVStorageDeviceVirtIO extends OMVStorageDeviceSMARTAbstract {
	protected $smartDeviceType = "sat";

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		$model = $this->getModel();
		return sprintf("%s [%s, %s]", !empty($model) ? $model :
		  gettext("VirtIO device"), $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}
}

/**
 * This class implements methods to get and process S.M.A.R.T.
 * AT Attachment (ATA) information and properties.
 * @ingroup api
 */
class OMVATASMARTInformation extends OMVObject {
	protected $deviceFile = "";
	protected $type = "";
	protected $sd = NULL;
	private $cmdOutput = NULL;
	private $dataCached = FALSE;
	private $attrDesc = array(
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
	);

	const SMART_ASSESSMENT_GOOD = "GOOD";
	const SMART_ASSESSMENT_BAD_ATTRIBUTE_IN_THE_PAST = "BAD_ATTRIBUTE_IN_THE_PAST";
	const SMART_ASSESSMENT_BAD_SECTOR = "BAD_SECTOR";
	const SMART_ASSESSMENT_BAD_ATTRIBUTE_NOW = "BAD_ATTRIBUTE_NOW";
	const SMART_ASSESSMENT_BAD_SECTOR_MANY = "BAD_SECTOR_MANY";
	const SMART_ASSESSMENT_BAD_STATUS = "BAD_STATUS";

	/**
	 * Constructor
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   </ul>
	 * @param type Specifies the type of the device, e.g. 'ata', 'cciss'
	 *   or '3ware'. For more details please check the option '-d' in
	 *   http://smartmontools.sourceforge.net/man5/smartd.conf.5.html.
	 * @param sd The storage device object. Defaults to NULL.
	 */
	public function __construct($deviceFile, $type, $sd = null) {
		$this->deviceFile = $deviceFile;
		$this->type = $type;
		$this->sd = $sd;
	}

	/**
	 * Get the device path, e.g. /dev/sda.
	 * @return The device path.
	 */
	public function getDeviceFile() {
		return $this->deviceFile;
	}

	/**
	 * Get the type of the device, e.g. 'ata', 'sat', 'cciss' or '3ware'.
	 * Note, this can be an empty string.
	 * @return The device type.
	 */
	public function getType() {
		return $this->type;
	}

	/**
	 * Get S.M.A.R.T. information.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;

		// Get all available S.M.A.R.T. information.
		$cmdArgs = array();
		$cmdArgs[] = sprintf("-x %s", escapeshellarg($this->deviceFile));
		if (!empty($this->type))
			$cmdArgs[] = sprintf("-d %s", $this->type);
		$cmd = sprintf("export LANG=C; smartctl %s", implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		// Bit 0: Command line did not parse
		// Bit 1: Device open failed, or device did not return an
		//        IDENTIFY DEVICE structure
		if (($result !== 0) && (($result & 0) || ($result & 1))) {
			$this->setLastError($output);
			return FALSE;
		}
		$this->cmdOutput = $output;

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the plain text 'smartctl' command output.
	 * @return The command output, otherwise FALSE.
	 */
	public function getExtendedInformation() {
		if ($this->getData() === FALSE)
			return FALSE;
		return implode("\n", $this->cmdOutput);
	}

	/**
	 * Get the S.M.A.R.T. attributes data structure.
	 * @return An array of S.M.A.R.T. attributes data, otherwise FALSE.
	 */
	public function getAttributes() {
		if ($this->getData() === FALSE)
			return FALSE;
		$result = array();
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
			$regex = '/^\s*(\d+)\s+(\S+)\s+([POSRCK-]+)\s+(\d+)\s+(\d+)\s+'.
			  '(\d+)\s+(\S+)\s+(.+)$/i';
			if (1 == preg_match($regex, $cmdOutputv, $matches)) {
				$id = intval($matches[1]);
				$attrData = array(
					"id" => $id,
					"attrname" => $matches[2],
					"flags" => $matches[3],
					"value" => intval($matches[4]),
					"worst" => intval($matches[5]),
					"treshold" => intval($matches[6]),
					"whenfailed" => $matches[7],
					"rawvalue" => $matches[8],
					"description" => array_key_exists($id, $this->attrDesc) ?
					  $this->attrDesc[$id] : "",
					// Additional fields used for the assessment.
					"prefailure" => (FALSE !== stripos($matches[3], "P")),
					"assessment" => self::SMART_ASSESSMENT_BAD_STATUS
				);
				// Assess prefailure attributes.
				if (TRUE === $attrData['prefailure'])
					$this->assessPrefailureAttribute($attrData);
				$result[] = $attrData;
			}
		}
		return $result;
	}

	private function assessPrefailureAttribute(&$attrData) {
		if (FALSE === $attrData['prefailure'])
			return;
		// Set the default assessment.
		$attrData['assessment'] = self::SMART_ASSESSMENT_GOOD;
		// Always-Fail and Always-Passing thresholds are not relevant
		// for our assessment.
		if (!(1 <= $attrData['treshold']) && (0xFD >= $attrData['treshold'])) {
			$attrData['assessment'] = self::SMART_ASSESSMENT_BAD_STATUS;
			return;
		}
		// See https://wiki.ubuntuusers.de/Festplattenstatus
		if ($attrData['value'] <= $attrData['treshold']) {
			$attrData['assessment'] = self::SMART_ASSESSMENT_BAD_ATTRIBUTE_NOW;
			return;
		}
		if ($attrData['worst'] <= $attrData['treshold']) {
			$attrData['assessment'] = self::SMART_ASSESSMENT_BAD_ATTRIBUTE_IN_THE_PAST;
			return;
		}
	}

	/**
	 * Get a specific attribute by ID.
	 * @return An object with the requested attribute data, otherwise FALSE.
	 */
	public function getAttribute($id) {
		if (FALSE === ($attributes = $this->getAttributes()))
			return FALSE;
		$result = FALSE;
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
	 * @return An array of S.M.A.R.T. self-test logs, otherwise FALSE.
	 */
	public function getSelfTestLogs() {
		if ($this->getData() === FALSE)
			return FALSE;
		$result = array();
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
	 * @return An array of strings, otherwise FALSE.
	 */
	public function getInformation() {
		if ($this->getData() === FALSE)
			return FALSE;
		// Initialize with default values. Note, the result list may
		// contain additional key/value pairs.
		$result = array(
			"devicemodel" => "",
			"serialnumber" => "",
			"firmwareversion" => "",
			"usercapacity" => ""
		);
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
		//
		// === START OF READ SMART DATA SECTION ===
		// ...
		$sectionFound = FALSE;
		foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
			$cmdOutputv = trim($cmdOutputv);
			// Abort parsing, we are not interested in the information
			// shown below this line.
			if ($cmdOutputv == "=== START OF READ SMART DATA SECTION ===")
				break;
			// Ignore everything that is not below this line.
			if ($cmdOutputv == "=== START OF INFORMATION SECTION ===") {
				$sectionFound = TRUE;
				continue;
			}
			// Have we found the information section?
			if (FALSE === $sectionFound)
				continue;
			// Parse the information section line:
			// Device Model:     WDC WD2502ABYS-02B7A0
			$regex = '/^([^:]+):\s+(.+)$/i';
			if (1 !== preg_match($regex, $cmdOutputv, $matches))
				continue;
			// Convert the attribute name, e.g. 'Device Model' to
			// 'devicemodel'.
			$attrKey = strtolower(str_replace(" ", "", $matches[1]));
			// Append key/value to result array.
			$result[$attrKey] = $matches[2];
		}
		return $result;
	}

	/**
	 * Get the device temperature in °C (value only, no unit).
	 * @return The temperature value, otherwise FALSE.
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
			case 231: // temperature-celsius
				// ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
				// 231 Temperature_Celsius     0x0013   100   100   010    Pre-fail  Always       -       0
				$regex = '/^(\d+)(\s+(.+))*$/';
				if (1 !== preg_match($regex, $attrv['rawvalue'], $matches))
					continue;
				// Verify temperature.
				$temp = intval($matches[1]);
				if ((-15 > $temp) || (100 < $temp))
					continue;
				if (!$found || ($temp > $result))
					$result = $temp;
				$found = TRUE;
				break;
			}
		}
		// If the SMART attributes are not present then it may be an older
		// SCSI device. Then the command output looks like:
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
		//
		// ...
		// SCT Status Version:                  3
		// SCT Version (vendor specific):       256 (0x0100)
		// SCT Support Level:                   1
		// Device State:                        DST executing in background (3)
		// Current Temperature:                    28 Celsius
		// Power Cycle Min/Max Temperature:     23/28 Celsius
		// Lifetime    Min/Max Temperature:     21/32 Celsius
		// ...
		if (FALSE === $found) {
			foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
				$regex = '/^(Current Drive Temperature|Current Temperature):'.
				  '\s+(\d+)\s+(C|Celsius)$/i';
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
	 * Get the overall assessment for the device.
	 * @return Returns the following strings:
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
		if (empty($attributes))
			return self::SMART_ASSESSMENT_BAD_STATUS;
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
			// Skip non-prefailure attributes.
			if (FALSE === $attrv['prefailure'])
				continue;
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
}

/**
 * Helper class that provides various functions regarding the systems network
 * configuration.
 * @ingroup api
 */
class OMVNetwork extends OMVObject {
	/**
	 * Get the systems host name.
	 * @param options Additional command parameters
	 * @return The host name if successful, otherwise FALSE.
	 */
	public static function getHostname($options) {
		$cmd = sprintf("export LANG=C; hostname %s", $options);
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return $output[0];
	}

	/**
	 * Get the systems domain name.
	 * @return The domain name if successful, otherwise FALSE.
	 */
	public static function getDomain() {
		$cmd = "export LANG=C; hostname -d";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return $output[0];
	}
}

/**
 * The generic class that represents a network interface backend.
 * @ingroup api
 */
abstract class OMVNetworkInterfaceBackendAbstract extends OMVObject {
	/**
	 * Get the type of the network interface backend, e.g.
	 * OMV_NETWORK_INTERFACE_TYPE_ETHERNET, OMV_NETWORK_INTERFACE_TYPE_BOND,
	 * OMV_NETWORK_INTERFACE_TYPE_VLAN, ...
	 */
	abstract function getType();

	/**
	 * Get a list of devices of the given network interface backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	abstract function enumerate();

	/**
	 * Get a list of all devices that are used by devices of this
	 * network interface backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	public function enumerateSlaves() {
		return array();
	}

	/**
	* Check whether the given device is represented by this network
	* interface backend.
	* @param deviceName Specifies the device name, e.g.
	*   <ul>
	*   \li eth1
	*   \li bond0
	*   \li eth0.1
	*   \li venet2
	*   </ul>
	* @return TRUE if successful, otherwise FALSE.
	*/
	public function isTypeOf($deviceName) {
		return FALSE;
	}

	/**
	 * Get the object of the class implementing the given network interface.
	 * @param args The arguments to the class constructor.
	 * @return The object of the class implementing the given network
	 *   interface type, otherwise NULL.
	 */
	public function getImpl($args) {
		return new OMVNetworkInterface($args);
	}

	/**
	 * Helper function to enumerate the network interfaces represented by
	 * this backend via the /sys filesystem.
	 * @param regex The regular expression used to identify the devices
	 *   represented by this backend.
	 * @return A list of network interface names, otherwise FALSE.
	 */
	final protected function enumerateSysFs($regex) {
		$result = array();
		$dir = new DirectoryIterator("/sys/class/net");
		foreach ($dir as $item) {
			// Skip everything that is not a symlink.
			if ($item->isDot() || !$item->isLink())
				continue;
			// Validate network interface name.
			if (1 !== preg_match($regex, $item->getFilename()))
				continue;
			$result[] = $item->getFilename();
		}
		return $result;
	}

	/**
	 * Helper function to check whether the given device is represented by
	 * this backend.
	 * @param deviceName Specifies the network interface name.
	 * @param regex The regular expression used to identify the devices
	 *   represented by this storage backend.
	 */
	final protected function isTypeOfByName($deviceName, $regex) {
		// Check if the network interface name matches the given regular
		// expression.
		return (1 == preg_match($regex, $deviceName));
	}
}

/**
 * Implements the network interface backend for ethernet devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendEthernet extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^(eth|venet)[0-9]+|veth[a-z0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_ETHERNET;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for Infiniband devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendInfiniband extends OMVNetworkInterfaceBackendEthernet {
	protected $regex = "/^ib[0-9]+$/i";
}

/**
 * Implements the network interface backend for bond devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendBond extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^bond[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_BOND;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}

	function enumerateSlaves() {
		if (FALSE === ($devs = $this->enumerate()))
			return FALSE;
		$result = array();
		foreach ($devs as $devk => $devv) {
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			$slaves = $object->getSlaves();
			if (FALSE === $slaves)
				continue;
			$result = array_merge($result, $slaves);
		}
		return $result;
	}

	function getImpl($args) {
		return new OMVNetworkInterfaceBond($args);
	}
}

/**
 * Implements the network interface backend for VLAN devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendVlan extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^(\S+\d+)\.(\d+)$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_VLAN;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for bridge devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendBridge extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^br[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_BRIDGE;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for bridge devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendWireless extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^wlan[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_WIRELESS;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for TUN/TAP devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendVirtual extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^(tun|tap)[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_VIRTUAL;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for loopback devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendLoopback extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^lo[0-9]*$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_LOOPBACK;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Helper class to get all existing Linux network interfaces.
 * @ingroup api
 */
class OMVNetworkInterfaces extends OMVObject {
	static private $backends = array();

	/**
	 * Register a storage device backend.
	 * @param backend The storage device backend object to register.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final static function registerBackend(OMVNetworkInterfaceBackendAbstract $backend) {
		if (!isset($backend))
			return FALSE;
		self::$backends[] = $backend;
		return TRUE;
	}

	/**
	 * Get the backend of the given network interface.
	 * @param deviceName Specifies the device name, e.g.
	 *   <ul>
	 *   \li eth0
	 *   \li wlan2
	 *   \li eth2.1
	 *   \li bond0
	 *   </ul>
	 * @return The backend that implements the given device, otherwise NULL.
	 */
	final static function getBackend($deviceName) {
		$result = NULL;
		foreach (self::$backends as $backendk => $backendv) {
			if (FALSE === $backendv->isTypeOf($deviceName))
				continue;
			$result = $backendv;
			break;
		}
		return $result;
	}

	/**
	 * Enumerate all network interfaces.
	 * @param type Defines the network interface type, e.g. ethernet, vlan or
	 *   bond. Defaults to OMV_NETWORK_INTERFACE_TYPE_ALL.
	 * @return A list of network interfaces, e.g. eth0 or lo, otherwise FALSE.
	 */
	public static function enumerate($type = OMV_NETWORK_INTERFACE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerate()))
				return FALSE;
			$result = array_unique(array_merge($result, $devs));
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Enumerate all unused devices. This list contains all devices of the
	 * given type except the devices that are used by other network interfaces
	 * (e.g. bond interfaces).
	 * @param type Defines the network interface type. Defaults to
	 *   OMV_NETWORK_INTERFACE_TYPE_ALL.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	final static function enumerateUnused($type = OMV_NETWORK_INTERFACE_TYPE_ALL) {
		// Get all existing network interfaces of the given type.
		if (FALSE === ($result = self::enumerate($type)))
			return FALSE;
		// Remove used network interfaces.
		foreach (self::$backends as $backendk => $backendv) {
// Always collect and remove all slave devices.
//			if (!($type & $backendv->getType()))
//				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_diff($result, $slaves);
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Enumerate all used devices. The list contains all those devices that
	 * are used by the given network interfaces, e.g. all members of a bond
	 * interface.
	 * @param type Defines the network interface type. Defaults to
	 *   OMV_NETWORK_INTERFACE_TYPE_ALL.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	final static function enumerateUsed($type = OMV_NETWORK_INTERFACE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_unique(array_merge($result, $slaves));
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Check if the given device is used by another network interface.
	 * @param deviceName Specifies the device name, e.g.
	 *   <ul>
	 *   \li eth0
	 *   \li wlan2
	 *   \li eth2.1
	 *   \li bond0
	 *   </ul>
	 * @param type Defines the network interface type. Defaults to
	 *   OMV_NETWORK_INTERFACE_TYPE_ALL.
	 * @return TRUE if the given device is used by another network
	 *   interface, otherwise FALSE.
	 */
	final static function isUsed($deviceName, $type = OMV_NETWORK_INTERFACE_TYPE_ALL) {
		$result = FALSE;
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (in_array($deviceName, $slaves)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the object of the class implementing the given filesystem.
	 * @param deviceName Specifies the device name, e.g.
	 *   <ul>
	 *   \li eth0
	 *   \li wlan2
	 *   \li eth2.1
	 *   \li bond0
	 *   </ul>
	 * @return The object of the class implementing the given network
	 *   interface, otherwise NULL.
	 */
	public static function getImpl($deviceName) {
		if (NULL == ($backend = self::getBackend($deviceName)))
			return NULL;
		return $backend->getImpl($deviceName);
	}
}

/**
 * This class provides a simple interface to handle Linux network interfaces.
 * @ingroup api
 */
class OMVNetworkInterface extends OMVObject {
	protected $name = "";
	protected $ifconfig = null;
	protected $ip = null;
	protected $ethtool = null;
	protected $regex = array(
		"ipv4" => '\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}',
		"ipv6" => '[a-f0-9:]+',
		"ipv6cidr" => '([a-f0-9:]+)\/(\d{1,2})',
		"state" => 'UP|DOWN|UNKNOWN'
	);

	/**
	 * Constructor
	 * @param name The network interface name, e.g. eth0, ethx, ...
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Get the network interface configuration
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData($type) {
		$result = FALSE;
		switch ($type) {
		case "ifconfig":
			if (!is_null($this->ifconfig))
				return TRUE;
			$cmd = sprintf("export LANG=C; ifconfig %s 2>&1",
			  escapeshellarg($this->getDeviceName()));
			@OMVUtil::exec($cmd, $output, $result);
			if (($result !== 0) || empty($output[0])) {
				$this->setLastError($output);
				return FALSE;
			}
			$this->ifconfig = implode("|", $output);
			$result = TRUE;
			break;
		case "ip":
			if (!is_null($this->ip))
				return TRUE;
			$cmd = sprintf("export LANG=C; ip addr show dev %s 2>&1",
			  escapeshellarg($this->getDeviceName()));
			@OMVUtil::exec($cmd, $output, $result);
			if (($result !== 0) || empty($output[0])) {
				$this->setLastError($output);
				return FALSE;
			}
			foreach ($output as $outputk => &$outputv)
				$outputv = trim($outputv);
			$this->ip = implode("|", $output);
			$result = TRUE;
			break;
		case "ethtool":
			if (!is_null($this->ethtool))
				return TRUE;
			$cmd = sprintf("export LANG=C; ethtool %s 2>&1",
			  escapeshellarg($this->getDeviceName()));
			@OMVUtil::exec($cmd, $output, $result);
			if (($result !== 0) || empty($output[0])) {
				$this->setLastError($output);
				return FALSE;
			}
			foreach ($output as $outputk => &$outputv)
				$outputv = trim($outputv);
			$this->ethtool = implode("|", $output);
			$result = TRUE;
			break;
		}
		return $result;
	}

	/**
	 * Get the network interface name, e.g. eth0 or ethx.
	 * @return The network interface name.
	 */
	public function getDeviceName() {
		return $this->name;
	}

	/**
	 * Check whether the network interface exists.
	 * @return TRUE if the network interface exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the network interface IPv4 address.
	 * @return The network interface IPv4 address, otherwise FALSE.
	 */
	public function getIP() {
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		if (1 !== preg_match("/inet addr:\s*({$this->regex['ipv4']})/i",
		  $this->ifconfig, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 address.
	 * @return The network interface IPv6 address, otherwise FALSE.
	 */
	public function getIP6() {
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		if (1 !== preg_match("/inet6 addr:\s*({$this->regex['ipv6cidr']})".
		  "\s*Scope:Global/i", $this->ifconfig, $matches))
			return FALSE;
		return $matches[2];
	}

	/**
	 * Get the network interface mask.
	 * @return The network interface mask, otherwise FALSE.
	 */
	public function getMask() {
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		if (1 !== preg_match("/Mask:\s*({$this->regex['ipv4']})/i",
		  $this->ifconfig, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 mask/prefix length.
	 * @return The network interface IPv6 mask/prefix length as integer,
	 *   otherwise FALSE.
	 */
	public function getMask6() {
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		if (1 !== preg_match("/inet6 addr:\s*({$this->regex['ipv6cidr']})".
		  "\s*Scope:Global/i", $this->ifconfig, $matches))
			return FALSE;
		return intval($matches[3]);
	}

	/**
	 * Get the network interface MAC address.
	 * @return The network interface MAC address, otherwise FALSE.
	 */
	public function getMAC() {
		$filename = sprintf("/sys/class/net/%s/address",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the network interface MTU.
	 * @return The network interface MTU, otherwise FALSE.
	 */
	public function getMTU() {
		$filename = sprintf("/sys/class/net/%s/mtu",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the network interface IPv4 default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 */
	public function getGateway() {
		$cmd = sprintf("export LANG=C; ip -4 route show dev %s 2>&1",
		  escapeshellarg($this->getDeviceName()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		$output = implode("\n", $output);
		// Parse command output:
		// 192.168.178.0/24  proto kernel  scope link  src 192.168.178.21
		// default via 192.168.178.1
		$regex = sprintf('/default via\s+(%s)/im', $this->regex['ipv4']);
		if (1 !== preg_match($regex, $output, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 */
	public function getGateway6() {
		$cmd = sprintf("export LANG=C; ip -6 route show dev %s 2>&1",
		  escapeshellarg($this->getDeviceName()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		$output = implode("\n", $output);
		// Parse command output:
		// fe80::/64  proto kernel  metric 256  mtu 1500 advmss 1440 hoplimit 4294967295
		// default via fe80::21e:13ff:fef9:af0  metric 1024  mtu 1500 advmss 1440 hoplimit 4294967295
		$regex = sprintf('/default via\s+(%s)\s+/im', $this->regex['ipv6']);
		if (1 !== preg_match($regex, $output, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface state.
	 * @return The network interface state, otherwise FALSE.
	 */
	public function getState() {
		if ($this->getData("ethtool") === FALSE)
			return FALSE;
		if (1 !== preg_match("/state {$this->regex['state']}/i",
		  $this->ethtool, $matches))
			return FALSE;
		return $matches[0];
	}

	/**
	 * Get the network interface link state.
	 * @return TRUE if link is established, otherwise FALSE.
	 */
	public function getLink() {
		if (FALSE === ($carrier = $this->getCarrier()))
			return FALSE;
		return (1 == $carrier) ? TRUE : FALSE;
	}

	/**
	 * Get the network interface statistics.
	 * @return The network interface statistics, otherwise FALSE.
	 * array(
	 *   rx_bytes => xxx,
	 *   rx_packets => xxx,
	 *   rx_errors => xxx,
	 *   rx_dropped => xxx,
	 *   rx_fifo_errors => xxx,
	 *   rx_frame_errors => xxx,
	 *   rx_compressed => xxx,
	 *   rx_multicast => xxx,
	 *   tx_bytes => xxx,
	 *   tx_packets => xxx,
	 *   tx_errors => xxx,
	 *   tx_dropped => xxx,
	 *   tx_fifo_errors => xxx,
	 *   tx_collisions => xxx,
	 *   tx_carrier_errors => xxx,
	 *   tx_compressed => xxx
	 * )
	 */
	public function getStatistics() {
		// Parse command output:
		// Inter-|   Receive                                                |  Transmit
		//  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
		//     lo:  156976     947    0    0    0     0          0         0   156976     947    0    0    0     0       0          0
		//   eth0:162137708  168681    0    0    0     0          0         0 15969317  103565    0    0    0     0       0          0
		//   eth1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		$regex = sprintf('/^\s*%s:(.*)$/im', $this->getDeviceName());
		if (1 !== preg_match($regex, file_get_contents("/proc/net/dev"),
		  $matches))
			return FALSE;
		$data = preg_split("/[\s]+/", $matches[1]);
		return array(
			  "rx_bytes" => $data[0],
			  "rx_packets" => $data[1],
			  "rx_errors" => $data[2],
			  "rx_dropped" => $data[3],
			  "rx_fifo_errors" => $data[4],
			  "rx_frame_errors" => $data[5],
			  "rx_compressed" => $data[6],
			  "rx_multicast" => $data[7],
			  "tx_bytes" => $data[8],
			  "tx_packets" => $data[9],
			  "tx_errors" => $data[10],
			  "tx_dropped" => $data[11],
			  "tx_fifo_errors" => $data[12],
			  "tx_collisions" => $data[13],
			  "tx_carrier_errors" => $data[14],
			  "tx_compressed" => $data[15]
		  );
	}

	/**
	 * Get the network interface operation state.
	 * @return The network interface operation state, otherwise FALSE.
	 */
	public function getOperState() {
		$filename = sprintf("/sys/class/net/%s/operstate",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the network interface duplex value.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface duplex value as string, otherwise FALSE.
	 */
	public function getDuplex() {
		$filename = sprintf("/sys/class/net/%s/duplex",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the network interface speed value.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface speed as number in Mbits/sec,
	 *   otherwise FALSE.
	 */
	public function getSpeed() {
		$filename = sprintf("/sys/class/net/%s/speed",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return intval(file_get_contents($filename));
	}

	/**
	 * Get the network interface physical link state.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface physical link state, otherwise FALSE.
	 */
	public function getCarrier() {
		$filename = sprintf("/sys/class/net/%s/carrier",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return intval(file_get_contents($filename));
	}

	/**
	 * Check if the network interface is a VLAN (802.1q) device.
	 * @return TRUE if the network interface is a VLAN (802.1q) device,
	 *   otherwise FALSE.
	 */
	public function isVlan() {
		$regex = "/^(\S+\d+)\.(\d+)$/i";
		if (1 !== preg_match($regex, $this->getDeviceName(), $matches))
			return FALSE;
		return TRUE;
	}

	/**
	* Get VLAN ID of the network interface.
	* @return The VLAN id, otherwise FALSE.
	*/
	public function getVlanId() {
		$regex = "/^(\S+\d+)\.(\d+)$/i";
		if (1 !== preg_match($regex, $this->getDeviceName(), $matches))
			return FALSE;
		return !empty($matches[2]) ? intval($matches[2]) : FALSE;
	}
}

/**
 * This class provides a interface to handle Linux bond network interfaces.
 * @ingroup api
 */
class OMVNetworkInterfaceBond extends OMVNetworkInterface {
	/**
	 * Get the slave devices.
	 * @return A list of device names used in the array, otherwise FALSE.
	 */
	public function getSlaves() {
		$filename = sprintf("/sys/class/net/%s/bonding/slaves",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		$slaves = file($filename);
		return $slaves;
	}
}

/**
 * Helper class to get the groups on the system.
 * @ingroup api
 */
class OMVUserGroups extends OMVObject {
	/**
	 * Get a list of system group names.
	 * @deprecated
	 * @return An array of system group names, otherwise FALSE.
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate system group names.
	 * @return An array of system group names, otherwise FALSE.
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; getent group 2>&1";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$list = array();
		// Parse command output:
		// shadow:x:42:openmediavault
		// snmp:x:112:
		// sambashare:x:113:
		// openmediavault:x:999:
		// nut:x:114:
		foreach ($output as $outputv) {
			$data = explode(":", $outputv);
			if (empty($data))
				continue;
			$list[] = $data[0];
		}
		return $list;
	}
}

/**
 * This class provides a simple interface to handle a system group.
 * @ingroup api
 */
class OMVUserGroup extends OMVObject {
	protected $name = null;
	protected $gid = null;
	protected $members = null;
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param id The name or GID of the system group.
	 */
	public function __construct($id) {
		if (is_int($id))
			$this->gid = $id;
		else
			$this->name = $id;
	}

	/**
	 * Get the system group data.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;
		// Query user information.
		$groupInfo = FALSE;
		if (!is_null($this->gid))
			$groupInfo = posix_getgrgid($this->gid);
		else
			$groupInfo = posix_getgrnam($this->name);
		if (FALSE === $groupInfo) {
			$this->setLastError(posix_strerror(posix_errno()));
			return FALSE;
		}
		$this->name = $groupInfo['name'];
		$this->gid = $groupInfo['gid'];
		$this->members = $groupInfo['members'];
		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;
		return TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Check whether the system group exists.
	 * @return TRUE if the system group exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
			return FALSE;
		return !is_null($this->name) && !is_null($this->gid);
	}

	/**
	 * Get the group name.
	 * @return The group name.
	 */
	public function getName() {
		if (!is_null($this->name))
			return $this->name;
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the group ID.
	 * @return The group ID, otherwise FALSE.
	 */
	public function getGid() {
		if (!is_null($this->gid))
			return $this->gid;
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->gid;
	}

	/**
	 * Get the user names who are member of this group.
	 * @return An array of user names that are member of this group,
	 * otherwise FALSE.
	 */
	public function getMembers() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->members;
	}

	/**
	 * Get the group quotas.
	 * @return An array containing the quotas.
	 */
	public function getQuotas() {
		$cmd = sprintf("export LANG=C; EDITOR=cat edquota -g %s 2>&1",
		  escapeshellarg($this->getName()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		// Parse command output:
		// Filesystem                   blocks       soft       hard     inodes     soft     hard
		// /dev/sdb1                     10188          0      12288          4        0        0
		// /dev/sdc1                         0          0      45056          0        0        0
		$result = array();
		foreach ($output as $outputk => $outputv) {
			if (preg_match("/^\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+".
			  "(\d+)\s+(\d+)$/i", $outputv, $matches)) {
				$result[$matches[1]] = array(
					"devicefile" => $matches[1],
					"blocks" => $matches[2],
					"bsoft" => $matches[3],
					"bhard" => $matches[4],
					"inodes" => $matches[5],
					"isoft" => $matches[6],
					"ihard" => $matches[7]
				);
			}
		}
		return $result;
	}

	/**
	 * Check if the given group is a system account.
	 * @return TRUE if the group is a system account, otherwise FALSE.
	 */
	public function isSystemAccount() {
		if (FALSE === ($gid = $this->getGid()))
			return FALSE;
		// Get shadow password suite configuration.
		$ld = \OMV\System\System::getLoginDefs();
		// Get the min/max values of the non-system account ID sequence.
		$min = array_key_exists("GID_MIN", $ld) ? $ld['GID_MIN'] : 1000;
		$max = array_key_exists("GID_MAX", $ld) ? $ld['GID_MAX'] : 60000;
		// Check if the given account ID is within the sequence.
		return !in_range($gid, $min, $max);
	}
}

/**
 * Helper class to get the users on the system.
 * @ingroup api
 */
class OMVUsers extends OMVObject {
	/**
	 * Get a list of system user names.
	 * @deprecated
	 * @return An array of system user names, otherwise FALSE.
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate system user names.
	 * @return An array of system user names, otherwise FALSE.
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; getent passwd";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$list = array();
		// Parse command output:
		// proftpd:x:109:65534::/var/run/proftpd:/bin/false
		// ftp:x:110:65534::/home/ftp:/bin/false
		// openmediavault:x:999:999::/home/openmediavault:/bin/sh
		// admin:x:998:100:WebGUI administrator:/home/admin:/usr/sbin/nologin
		// nut:x:111:114::/var/lib/nut:/bin/false
		// test:x:1001:100:sdfds:/home/test:/bin/dash
		foreach ($output as $outputv) {
			$data = explode(":", $outputv);
			if (empty($data))
				continue;
			$list[] = $data[0]; // User name
		}
		return $list;
	}
}

/**
 * This class provides a simple interface to handle a system user.
 * @ingroup api
 */
class OMVUser extends OMVObject {
	protected $name = null;
	protected $uid = null;
	protected $gid = null;
	protected $password = null;
	protected $gecos = null;
	protected $dir = null;
	protected $shell = null;
	protected $lastchanged = null;
	protected $minimum = null;
	protected $maximum = null;
	protected $warn = null;
	protected $inactive = null;
	protected $expire = null;
	protected $reserved = null;
	protected $groups = null;

	/**
	 * Constructor
	 * @param id The name or UID of the system user.
	 */
	public function __construct($id) {
		if(is_int($id))
			$this->uid = $id;
		else
			$this->name = $id;
	}

	/**
	 * Get the system group data.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData($type) {
		$result = FALSE;
		switch($type) {
		case "shadow":
			if (!is_null($this->password))
				return TRUE;
			// http://www.cyberciti.biz/faq/understanding-etcshadow-file
			$cmd = sprintf("export LANG=C; getent shadow %s 2>&1",
			  escapeshellarg($this->getName()));
			@OMVUtil::exec($cmd, $output, $result);
			if (($result !== 0) || empty($output[0])) {
				$this->setLastError($output);
				return FALSE;
			}
			$output = explode(":", $output[0]);
			$this->password = $output[1];
			$this->lastchanged = $output[2];
			$this->minimum = $output[3];
			$this->maximum = $output[4];
			$this->warn = $output[5];
			$this->inactive = $output[6];
			$this->expire = $output[7];
			$this->reserved = $output[8];
			$result = TRUE;
			break;
		case "passwd":
			if (!is_null($this->name) && !is_null($this->uid))
				return TRUE;
			// Query user information.
			$userInfo = FALSE;
			if (!is_null($this->uid))
				$userInfo = posix_getpwuid($this->uid);
			else
				$userInfo = posix_getpwnam($this->name);
			if (FALSE === $userInfo) {
				$this->setLastError(posix_strerror(posix_errno()));
				return FALSE;
			}
			$this->name = $userInfo['name'];
			$this->uid = $userInfo['uid'];
			$this->gid = $userInfo['gid'];
			$this->gecos = $userInfo['gecos'];
			$this->dir = $userInfo['dir'];
			$this->shell = $userInfo['shell'];
			$result = TRUE;
			break;
		case "groups":
			if (!is_null($this->groups))
				return TRUE;
			// Get the group IDs the user is in as white-space separated
			// numbers. Thus it is possible to correctly process group
			// names that contain blanks (e.g. imported via LDAP or AD).
			$cmd = sprintf("export LANG=C; id -G %s 2>&1",
			  escapeshellarg($this->getName()));
			@OMVUtil::exec($cmd, $output, $result);
			if (($result !== 0) || empty($output[0])) {
				$this->setLastError($output);
				return FALSE;
			}
			$gids = explode(" ", $output[0]);
			// Resolve the group names.
			$this->groups = array();
			foreach ($gids as $gidk => $gidv) {
				$groupInfo = posix_getgrgid($gidv);
				$this->groups[] = $groupInfo['name'];
			}
			$result = TRUE;
			break;
		}
		return $result;
	}

	/**
	 * Check whether the system user exists.
	 * @return TRUE if the system user exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData("passwd") === FALSE)
			return FALSE;
		return !is_null($this->name) && !is_null($this->uid);
	}

	/**
	 * Get the user name.
	 * @return The user name.
	 */
	public function getName() {
		if (!is_null($this->name))
			return $this->name;
		if ($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the user ID.
	 * @return The user ID, otherwise FALSE.
	 */
	public function getUid() {
		if (!is_null($this->uid))
			return $this->uid;
		if ($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->uid;
	}

	/**
	 * Get the users group ID.
	 * @return The user group ID, otherwise FALSE.
	 */
	public function getGid() {
		if ($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->gid;
	}

	/**
	 * Get the comment.
	 * @return The comment, otherwise FALSE.
	 */
	public function getGecos() {
		if ($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->gecos;
	}

	/**
	 * Get the home directory.
	 * @return The home directory, otherwise FALSE.
	 */
	public function getHomeDirectory() {
		if ($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->dir;
	}

	/**
	 * Get the shell.
	 * @return The shell, otherwise FALSE.
	 */
	public function getShell() {
		if ($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->shell;
	}

	/**
	 * Get the encrypted password.
	 * @return The encrypted password, otherwise FALSE.
	 */
	public function getPassword() {
		if ($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->password;
	}

	public function getLastChanged() {
		if ($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->lastchanged;
	}

	public function getMinimum() {
		if ($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->minimum;
	}

	public function getMaximum() {
		if ($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->maximum;
	}

	public function getWarn() {
		if ($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->warn;
	}

	public function getInactive() {
		if ($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->inactive;
	}

	public function getExpire() {
		if ($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->expire;
	}

	public function getReserved() {
		if ($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->reserved;
	}

	/**
	 * Get the groups the user is in.
	 * @return An array of groups the user is in, otherwise FALSE.
	 */
	public function getGroups() {
		if ($this->getData("groups") === FALSE)
			return FALSE;
		return $this->groups;
	}

	/**
	 * Get the user quotas.
	 * @return An array containing the quotas.
	 */
	public function getQuotas() {
		$cmd = sprintf("export LANG=C; EDITOR=cat edquota -u %s 2>&1",
		  escapeshellarg($this->getName()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		// Parse command output:
		// Filesystem                   blocks       soft       hard     inodes     soft     hard
		// /dev/sdb1                     10188          0      12288          4        0        0
		// /dev/sdc1                         0          0      45056          0        0        0
		$result = array();
		foreach ($output as $outputk => $outputv) {
			if (preg_match("/^\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+".
			  "(\d+)\s+(\d+)$/i", $outputv, $matches)) {
				$result[$matches[1]] = array(
					"devicefile" => $matches[1],
					"blocks" => $matches[2],
					"bsoft" => $matches[3],
					"bhard" => $matches[4],
					"inodes" => $matches[5],
					"isoft" => $matches[6],
					"ihard" => $matches[7]
				);
			}
		}
		return $result;
	}

	/**
	 * Authenticate user via PAM. Account expiration and access hours are
	 * checked, too. Note, the calling process must have the privilege to
	 * read the user shadow file.
	 * @param password The password.
	 * @return TRUE if the authentication was successful, otherwise FALSE.
	 */
	public function authenticate($password) {
		$error = "";
		if (FALSE === pam_auth($this->getName(), $password, $error, TRUE)) {
			$this->setLastError($error);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Check if the given user is a system account.
	 * @return TRUE if the user is a system account, otherwise FALSE.
	 */
	public function isSystemAccount() {
		if (FALSE === ($uid = $this->getUid()))
			return FALSE;
		// Get shadow password suite configuration.
		$ld = \OMV\System\System::getLoginDefs();
		// Get the min/max values of the non-system account ID sequence.
		$min = array_key_exists("UID_MIN", $ld) ? $ld['UID_MIN'] : 1000;
		$max = array_key_exists("UID_MAX", $ld) ? $ld['UID_MAX'] : 60000;
		// Check if the given account ID is within the sequence.
		return !in_range($uid, $min, $max);
	}
}

/**
 * This class provides a simple interface to handle a LVM physical volume.
 * @ingroup api
 */
class OMVLvmPhysicalVolumes extends OMVObject {
	/**
	 * Get a list of physical volumes.
	 * @deprecated
	 * @return A list of physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate LVM physical volumes.
	 * @return A list of LVM physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; pvdisplay --noheadings -C -o pv_name ".
		  "2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		$list = array();
		// Parse command output:
		//   /dev/sdc
		//   /dev/sdd
		//   unknown device
		foreach($output as $outputk => $outputv) {
			$deviceFile = trim($outputv);
			if(!is_devicefile($deviceFile))
				continue;
			$list[] = $deviceFile;
		}
		return $list;
	}
}

/**
 * Helper class to get the LVM logical volumes.
 * @ingroup api
 */
class OMVLvmLogicalVolumes extends OMVObject {
	/**
	 * Get a list of logical volumes.
	 * @deprecated
	 * @return A list of logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate LVM logical volumes.
	 * @return A list of LVM logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; lvdisplay --noheadings --separator '|' ".
		  "-C -o vg_name,lv_name 2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		$list = array();
		foreach($output as $outputk => $outputv) {
			$outputv = explode("|", trim($outputv));
			// Replace '-' with '--' in volume group name.
			$list[] = sprintf("/dev/mapper/%s-%s",
			  str_replace("-", "--", $outputv[0]),
			  str_replace("-", "--", $outputv[1]));
			// Alternative device path: /dev/<vg_name>/<lv_name>
			//$list[] = sprintf("/dev/%s/%s", $outputv[0], $outputv[1]);
		}
		return $list;
	}
}

/**
 * This class provides a simple interface to handle a LVM logical volume.
 * @ingroup api
 */
class OMVLvmLogicalVolume extends OMVStorageDeviceDM {
	protected $uuid = "";
	protected $attr = "";
	protected $kernelMajor = "";
	protected $kernelMinor = "";
	protected $lvName = "";
	protected $vgName = "";
	private $dataCached = FALSE;

	/**
	 * Get the logical volume detailed information.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;

		// Parse command output:
		// VOSP6y-Lw75-pbTs-3v8A-6Qek-XBKJ-Md2G1q|vltest|test|12582912B
		$cmd = sprintf("export LANG=C; lvdisplay --noheadings ".
		  "--separator '|' -C -o lv_uuid,lv_name,vg_name,lv_size,lv_attr,".
		  "lv_kernel_major,lv_kernel_minor --unit b %s",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}

		$output = explode("|", trim($output[0]));

		$this->uuid = $output[0];
		$this->lvName = $output[1];
		$this->vgName = $output[2];
		$this->size = substr($output[3], 0, -1);
		$this->attr = $output[4];
		$this->kernelMajor = intval($output[5]);
		$this->kernelMinor = intval($output[6]);

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the logical volume exists.
	 * @return TRUE if the logical volume exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
			return FALSE;
		return !empty($this->uuid);
	}

	/**
	 * Get the name of the volume group.
	 * @return The volume group name, FALSE on failure.
	 */
	public function getVGName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->vgName;
	}

	/**
	 * Get the name of the logical volume.
	 * @return The logical volume name, e.g. lvol0, or FALSE on failure.
	 */
	public function getName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->lvName;
	}

	/**
	 * Get the path of the logical volume.
	 * @return The logical volume path, e.g. /dev/vg0/lvol0, or FALSE on
	 * failure.
	 */
	public function getPath() {
		if ($this->getData() === FALSE)
			return FALSE;
		return build_path(array("/dev", $this->getVGName(), $this->getName()));
	}

	/**
	 * Get the size of the logical volume in bytes.
	 * @return The size of the logical volume in bytes as string,
	 * FALSE on failure.
	 */
	public function getSize() {
		if ($this->getData() === FALSE)
			return FALSE;
		return parent::getSize();
	}

	/**
	 * Get the UUID of the array.
	 * @return The UUID of the array, FALSE on failure.
	 */
	public function getUuid() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * Get the description of the logical volume.
	 * @return The logical volume description, FALSE on failure.
	 */
	public function getDescription() {
		if ($this->getData() === FALSE)
			return FALSE;
		return sprintf(gettext("LVM logical volume %s [%s, %s]"),
		  $this->getName(), $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}

	/**
	 * Create the logical volume.
	 * @param name The name for the new logical volume.
	 * @param size Gives the size of bytes to allocate for the new
	 *   logical volume.
	 * @param vgName The name of the volume group where to create the
	 *   logical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function create($name, $size, $vgName) {
		$cmd = sprintf("export LANG=C; lvcreate --name %s --size %sK %s 2>&1",
		  escapeshellarg($name), binary_convert($size, "B", "KiB"),
		  escapeshellarg($vgName));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Remove the logical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function remove() {
		$cmd = sprintf("export LANG=C; lvremove --force %s 2>&1",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Rename the logical volume.
	 * @param name The new logical volume path/name.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function rename($name) {
		if ($this->getData() === FALSE)
			return FALSE;
		$cmd = sprintf("export LANG=C; lvrename %s %s 2>&1", escapeshellarg(
		  $this->getPath()), escapeshellarg($name));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Extend the logical volume.
	 * @param size Gives the size of bytes to extend the logical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function extend($size) {
		// Convert size to KiB to ensure it is a multiple of 512.
		$cmd = sprintf("export LANG=C; lvextend --size %sK %s 2>&1",
		  binary_convert($size, "B", "KiB"), escapeshellarg(
		  $this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Reduce the logical volume.
	 * @param size Gives the size of bytes to extend the logical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function reduce($size) {
		// Convert size to KiB to ensure it is a multiple of 512.
		$cmd = sprintf("export LANG=C; lvreduce --size %sK %s 2>&1",
		  binary_convert($size, "B", "KiB"), escapeshellarg(
		  $this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}
}
