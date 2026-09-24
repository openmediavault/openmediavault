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

namespace OMV\System\Storage;

require_once("openmediavault/functions.inc");

/**
 * This class provides a simple interface to handle SCSI disk devices.
 * @ingroup api
 */
class StorageDeviceSD extends StorageDevice implements SmartInterface
{
    use SmartTrait;

    public function getSerialNumber()
    {
        if (true === $this->hasUdevProperty("ID_SCSI_SERIAL")) {
            return $this->getUdevProperty("ID_SCSI_SERIAL");
        }
        return parent::getSerialNumber();
    }

    /**
     * See \OMV\System\Storage\SmartInterface interface definition.
     */
    public function getSmartDeviceType()
    {
        if (true === $this->isUsb()) {
            // Identify by ID_MODEL_ID or `/sys/block/<XXX>/device/model`.
            $modelMap = [
                "TR-004 DISK00" => "jmb39x-q,0", // QNAP TR-004
                "TR-004 DISK01" => "jmb39x-q,1",
                "TR-004 DISK02" => "jmb39x-q,2",
                "TR-004 DISK03" => "jmb39x-q,3",
                "TR-004U DISK00" => "jmb39x-q,0", // QNAP TR-004U
                "TR-004U DISK01" => "jmb39x-q,1",
                "TR-004U DISK02" => "jmb39x-q,2",
                "TR-004U DISK03" => "jmb39x-q,3"
            ];
            return array_value($modelMap, $this->getModel(), "");
        }
        return "";
    }

    /**
     * Get the SCSI address of the device.
     *
     * The following fields are included:
     * - HBA number [host]
     * - Channel on the HBA [channel]
     * - SCSI target ID [target]
     * - LUN [lun]
     *
     * See https://tldp.org/HOWTO/SCSI-2.4-HOWTO/scsiaddr.html
     *
     * @return array Returns an object containing the host, bus, target
     *   and lun.
     * @throws \OMV\Exception
     */
    public function getHCTL(): array
    {
        // The path '/sys/<DEVPATH>/device/' points to the necessary information.
        // Examples:
        // $ ls -alh /sys/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0/block/sda
        // lrwxrwxrwx  1 root root    0 Feb  6 08:26 device -> ../../../2:0:1:0
        // $ ls -alh /sys/devices/pci0000:00/0000:00:02.2/0000:02:00.0/host1/scsi_host/host1/port-1:6/end_device-1:6/target1:0:5/1:0:5:0/block/sde
        // lrwxrwxrwx  1 root root    0 Oct  12 10:45 device -> ../../../1:0:5:0
        $path = realpath(build_path(
            DIRECTORY_SEPARATOR,
            "/sys",
            $this->getUdevProperty("DEVPATH"),
            "device/"
        ));
        if (false === $path) {
            throw new \OMV\Exception(
                "Failed to resolve the SCSI device path for '%s'.",
                $this->getDeviceFile()
            );
        }
        $parts = explode(":", basename($path));
        if (4 !== count($parts)) {
            throw new \OMV\Exception(
                "Failed to parse the SCSI address from path '%s'.",
                $path
            );
        }
        return [
            "host" => intval($parts[0]),
            "channel" => intval($parts[1]),
            "target" => intval($parts[2]),
            "lun" => intval($parts[3])
        ];
    }

    /**
     * Get the driver name of the host device this storage device is
     * connected to, e.g. 'hpsa', 'arcmsr' or 'ahci'.
     * @return Returns the driver name of the host device or an empty
     *   string.
     */
    public function getHostDriver(): string
    {
        // Try to get the driver via 'driver'.
        $hostPath = sprintf(
            "/sys/block/%s/device/../..",
            $this->getDeviceName(true)
        );
        $driverPath = realpath(sprintf("%s/../driver", $hostPath));
        if (file_exists($driverPath)) {
            return basename($driverPath);
        }
        // Try to get the driver via /sys/class/scsi_host/hostN/proc_name.
        // 'proc_name' is the "name of proc directory" of a driver, if
        // the driver maintained one.
        try {
            $hctl = $this->getHCTL();
        } catch (\OMV\Exception $e) {
            return "";
        }
        $procNamePath = sprintf(
            "/sys/class/scsi_host/host%d/proc_name",
            $hctl['host']
        );
        if (file_exists($procNamePath)) {
            return trim(file_get_contents($procNamePath));
        }
        return "";
    }

    private function isSAS(): bool
    {
        // drwxr-xr-x 8 root root    0 Jul  1 17:15 .
        // drwxr-xr-x 4 root root    0 Jul  1 17:15 ..
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 access_state
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 blacklist
        // drwxr-xr-x 3 root root    0 Jul  1 17:15 block
        // drwxr-xr-x 3 root root    0 Jul  1 17:15 bsg
        // --w------- 1 root root 4.0K Jul  4 08:49 delete
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 device_blocked
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 device_busy
        // -rw-r--r-- 1 root root 4.0K Jul  4 08:49 dh_state
        // lrwxrwxrwx 1 root root    0 Jul  4 08:49 driver -> ../../../../../../../../../..                                                                                                  /bus/scsi/drivers/sd
        // ...
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 modalias
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 model
        // ...
        // --w------- 1 root root 4.0K Jul  4 08:49 rescan
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 rev
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 sas_address
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 sas_device_handle
        // -rw-r--r-- 1 root root 4.0K Jul  4 08:49 sas_ncq_prio_enable
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 sas_ncq_prio_supported
        // drwxr-xr-x 3 root root    0 Jul  1 17:15 scsi_device
        // drwxr-xr-x 3 root root    0 Jul  1 17:15 scsi_disk
        // drwxr-xr-x 3 root root    0 Jul  1 17:15 scsi_generic
        // -r--r--r-- 1 root root 4.0K Jul  4 08:49 scsi_level
        $filename = sprintf(
            "/sys/block/%s/device/sas_address",
            $this->getDeviceName()
        );
        if (file_exists($filename)) {
            return true;
        }
        // E: ID_PATH=pci-0000:00:10.0-sas-exp0x5001438035ab31bd-phy18-lun-0
        if (true === $this->hasUdevProperty("ID_PATH")) {
            $property = $this->getUdevProperty("ID_PATH");
            if (1 == preg_match('/^.+-sas-.+$/i', $property)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Check if the device is connected to an external or hot-plug capable
     * SATA (eSATA) port.
     *
     * References:
     * - Serial ATA Advanced Host Controller Interface (AHCI) 1.3.1 Spec:
     *   - Section 3.1.1 (Offset 00h: CAP - Host Capabilities, bit 5 SXS)
     *   - Section 3.3.7 (Offset 18h: PxCMD - Port Command and Status,
     *     bit 18 HPCP, bit 21 ESP)
     * - Linux kernel drivers/ata/ahci.c: ahci_mark_external_port()
     *
     * @return bool Returns TRUE if the SATA port is hot-plug capable or
     *   external, otherwise FALSE.
     */
    private function isAhciHotPluggable(): bool
    {
        if (false === $this->isAta()) {
            return false;
        }
        try {
            $hctl = $this->getHCTL();
        } catch (\OMV\Exception $e) {
            return false;
        }
        $portCmdFile = sprintf(
            "/sys/class/scsi_host/host%d/ahci_port_cmd",
            $hctl['host']
        );
        if (!file_exists($portCmdFile)) {
            return false;
        }
        $portCmdContent = file_get_contents($portCmdFile);
        if (false === $portCmdContent) {
            return false;
        }
        $portCmd = hexdec(trim($portCmdContent));
        // Check if the port is hot-plug capable (PORT_CMD_HPCP = BIT(18) = 0x40000).
        if ($portCmd & 0x40000) {
            return true;
        }
        // Check if the port is an external SATA port (PORT_CMD_ESP = BIT(21) = 0x200000).
        // According to AHCI spec, ESP requires controller support (HOST_CAP_SXS = BIT(5) = 0x20).
        if ($portCmd & 0x200000) {
            $hostCapsFile = sprintf(
                "/sys/class/scsi_host/host%d/ahci_host_caps",
                $hctl['host']
            );
            if (file_exists($hostCapsFile)) {
                $hostCapsContent = file_get_contents($hostCapsFile);
                if (false !== $hostCapsContent) {
                    $hostCaps = hexdec(trim($hostCapsContent));
                    if ($hostCaps & 0x20) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public function isHotPluggable(): bool
    {
        return $this->isSAS() || $this->isAhciHotPluggable() ||
            parent::isHotPluggable();
    }
}
