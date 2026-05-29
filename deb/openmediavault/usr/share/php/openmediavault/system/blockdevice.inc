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
 * This class implements an interface to a generic Linux block device.
 * @ingroup api
 */
class BlockDevice implements BlockDeviceInterface, UdevInterface
{
    use UdevTrait;

    protected $deviceFile = null;
    protected $deviceFileById = null;
    protected $size = false;
    protected $blockSize = false;
    protected $sectorSize = false;

    /**
     * Constructor
     * @param deviceFile The device file, e.g.
     *   <ul>
     *   \li /dev/sda2
     *   \li /dev/sdb
     *   \li /dev/md1
     *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
     *   \li /dev/disk/by-id/wwn-0x5000cca211cc703c-part1
     *   \li /dev/disk/by-id/md-name-vmpc01:data
     *   \li /dev/disk/by-id/md-uuid-75de9de9:6beca92e:8442575c:73eabbc9
     *   \li /dev/disk/by-id/lvm-pv-uuid-VDH5Om-Rkjc-cQid-PeJI-Sfqm-66DI-w0dpCO
     *   </ul>
     * @throws \InvalidArgumentException
     */
    public function __construct($deviceFile)
    {
        if (true === is_devicefile_by_id($deviceFile)) {
            $this->deviceFileById = $deviceFile;
            $this->deviceFile = realpath($deviceFile);
        } elseif (true === is_devicefile($deviceFile)) {
            $this->deviceFile = $deviceFile;
        } else {
            throw new \InvalidArgumentException(sprintf(
                "Invalid device file '%s'.",
                strval($deviceFile)
            ));
        }
    }

    /**
     * See interface definition.
     */
    public function exists()
    {
        return is_block_device($this->getDeviceFile());
    }

    /**
     * Assert that the device exists.
     * @return void
     * @throw \OMV\AssertException
     */
    public function assertExists()
    {
        if (false === $this->exists()) {
            throw new \OMV\AssertException(
                "Device '%s' does not exist.",
                $this->getDeviceFile()
            );
        }
    }

    /**
     * See interface definition.
     */
    public function getDeviceFile()
    {
        return $this->deviceFile;
    }

    /**
     * See interface definition.
     */
    public function getCanonicalDeviceFile()
    {
        $deviceFile = realpath($this->deviceFile);
        if (false === $deviceFile) {
            throw new \OMV\Exception(
                "Device '%s' does not exist.",
                $this->deviceFile
            );
        }
        return $deviceFile;
    }

    /**
     * See interface definition.
     */
    public function getDeviceFileById()
    {
        if (false === is_devicefile_by_id($this->deviceFileById)) {
            // Get all device file symlinks.
            $symlinks = $this->getDeviceFileSymlinks();
            // Get the 'by-id' device file symlinks.
            $deviceNamesById = [];
            $regex = "/^\/dev\/disk\/by-id\/(.+)$/";
            foreach ($symlinks as $symlinkk => $symlinkv) {
                if (1 == preg_match($regex, $symlinkv, $matches)) {
                    $deviceNamesById[] = $matches[1];
                }
            }
            if (true === empty($deviceNamesById)) {
                return null;
            }
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
            // lvm-pv-uuid-VDH5Om-Rkjc-cQid-PeJI-Sfqm-66DI-w0dpCO
            // xxx...
            // yyy...
            //
            // Use a decorate-sort-undecorate method to sort the list of
            // device names.
            array_walk($deviceNamesById, function (&$v, $k) {
                $map = array("a" => 0, "w" => 1, "s" => 2);
                $v = array($v, array_key_exists($v[0], $map) ?
                    $map[$v[0]] : 3);
            });
            usort($deviceNamesById, function ($a, $b) {
                return ($a[1] < $b[1]) ? -1 : (($a[1] > $b[1]) ? 1 :
                    strnatcmp($a[0], $b[0]));
            });
            array_walk($deviceNamesById, function (&$v, $k) {
                $v = $v[0];
            });
            // Finally build the whole path of the device file.
            $this->deviceFileById = sprintf(
                "/dev/disk/by-id/%s",
                array_shift($deviceNamesById)
            );
        }
        return $this->deviceFileById;
    }

    /**
     * See interface definition.
     */
    public function hasDeviceFileById()
    {
        return is_devicefile_by_id($this->getDeviceFileById());
    }

    /**
     * See interface definition.
     */
    public function getDeviceFileByPath()
    {
        $result = null;
        // Get all device file symlinks.
        $symlinks = $this->getDeviceFileSymlinks();
        foreach ($symlinks as $symlinkk => $symlinkv) {
            if (true === is_devicefile_by_path($symlinkv)) {
                $result = $symlinkv;
                break;
            }
        }
        return $result;
    }

    /**
     * See interface definition.
     */
    public function hasDeviceFileByPath()
    {
        return is_devicefile_by_path($this->getDeviceFileByPath());
    }

    /**
     * See interface definition.
     */
    public function getPredictableDeviceFile()
    {
        if (true === $this->hasDeviceFileById()) {
            return $this->getDeviceFileById();
        }
        if (true === $this->hasDeviceFileByPath()) {
            return $this->getDeviceFileByPath();
        }
        return $this->getCanonicalDeviceFile();
    }

    /**
     * See interface definition.
     */
    public function getPreferredDeviceFile()
    {
        return $this->getCanonicalDeviceFile();
    }

    /**
     * See interface definition.
     */
    public function getDeviceFileSymlinks()
    {
        if (false === $this->hasUdevProperty("DEVLINKS")) {
            return [];
        }
        $property = $this->getUdevProperty("DEVLINKS");
        // Parse the property value, e.g.:
        // - disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1 disk/by-path/pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0-part1 disk/by-uuid/3849-705A
        // - /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
        // - /dev/disk/by-id/ata-ST1000DM003-1CH132_S2DF60PB /dev/disk/by-id/wwn-0x5000b5016caa4832
        // - /dev/mapper/omv--lvm--vg-root /dev/omv-lvm-vg/root /dev/disk/by-id/dm-name-omv--lvm--vg-root /dev/disk/by-id/dm-uuid-LVM-qrB1zl0Sshr0gzosPHD4NeJC7D2aLNF50m40r7lb7304SjsnCw6V0hOOeMmByPrq /dev/disk/by-uuid/4a1aa5db-980b-43ac-a62a-a2b9eadcb34f
        $parts = array_map("trim", explode(" ", $property));
        sort($parts, SORT_NATURAL);
        $canonicalDeviceFile = $this->getCanonicalDeviceFile();
        $result = [];
        foreach ($parts as $partk => $partv) {
            // Make sure that the device path is correct.
            if (true === is_devicefile($partv)) {
                $deviceFile = $partv;
            } else {
                $deviceFile = sprintf("/dev/%s", $partv);
            }
            // Make sure the symlink points to the canonical device file.
            // This prevents returning symlinks that have been "hijacked" by
            // DM multipath devices or other layers.
            if (realpath($deviceFile) === $canonicalDeviceFile) {
                $result[] = $deviceFile;
            }
        }
        return $result;
    }

    /**
     * See interface definition.
     */
    public function getDeviceName($canonical = false)
    {
        // Get the device file and extract the name, e.g. /dev/sda => sda.
        $deviceName = str_replace("/dev/", "", !$canonical ?
            $this->getDeviceFile() : $this->getCanonicalDeviceFile());
        return $deviceName;
    }

    /**
     * See interface definition.
     */
    public function getSize()
    {
        if (false === $this->size) {
            // The `/sys/block/<disk>/size` file reports the total size of a
            // block device in 512-byte sectors, regardless of the hardware's
            // physical block size.
            $this->size = bcmul($this->getSysfsValue("size"), "512", 0);
        }
        return $this->size;
    }

    /**
     * See interface definition.
     */
    public function getBlockSize()
    {
        if (false === $this->blockSize) {
            $cmdArgs = [];
            $cmdArgs[] = "--getbsz";
            $cmdArgs[] = escapeshellarg($this->getDeviceFile());
            $cmd = new \OMV\System\Process("blockdev", $cmdArgs);
            $cmd->setRedirect2to1();
            $cmd->setRequireOutput();
            $cmd->execute($output);
            $this->blockSize = intval($output[0]);
        }
        return $this->blockSize;
    }

    /**
     * See interface definition.
     */
    public function getSectorSize()
    {
        if (false === $this->sectorSize) {
            $this->sectorSize = intval($this->getSysfsValue(
                "queue/logical_block_size"
            ));
        }
        return $this->sectorSize;
    }

    /**
     * See interface definition.
     */
    public function getDeviceNumber()
    {
        try {
            return $this->getSysfsValue("dev");
        } catch (\OMV\FileException $e) {
            return false;
        }
    }

    /**
     * See interface definition.
     */
    public function getMajor()
    {
        if (false === ($devNum = $this->getDeviceNumber())) {
            return false;
        }
        $devNumParts = explode(":", $devNum);
        return intval($devNumParts[0]);
    }

    /**
     * See interface definition.
     */
    public function getMinor()
    {
        if (false === ($devNum = $this->getDeviceNumber())) {
            return false;
        }
        $devNumParts = explode(":", $devNum);
        return intval($devNumParts[1]);
    }

    /**
     * See interface definition.
     */
    public function getDescription()
    {
        return sprintf(
            "Block device %s [%s]",
            $this->getDeviceName(),
            $this->getDeviceNumber()
        );
    }

    /**
     * See interface definition.
     */
    public function waitForDevice($timeout)
    {
        waitUntil($timeout, [$this, "exists"], sprintf(
            "Device '%s' is not available after a waiting period of %d seconds.",
            $this->getDeviceFile(),
            $timeout
        ));
    }

    /**
     * Read a value from the block device sysfs subtree.
     * @param string $attributePath The path below the block sysfs subtree,
     *   e.g. `queue/logical_block_size`.
     * @return string The trimmed value.
     * @throws \OMV\FileException If the sysfs file does not exist.
     * @access protected
     */
    protected function getSysfsValue($attributePath)
    {
        $filename = build_path(
            DIRECTORY_SEPARATOR,
            "/sys/class/block",
            $this->getDeviceName(true),
            $attributePath
        );
        if (!file_exists($filename)) {
            throw new \OMV\FileException(
                "File '%s' does not exist.",
                $filename
            );
        }
        return trim(file_get_contents($filename));
    }
}
