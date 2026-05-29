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
 * @ingroup api
 */
class StorageDevice extends \OMV\System\BlockDevice implements StorageDeviceInterface
{
    /**
     * Get the device model (via udev or sysfs).
     * @return string The device model, otherwise an empty string.
     *   Note, underscores are replaced by blanks when the value is
     *   retrieved via udev.
     */
    public function getModel()
    {
        if (false === $this->hasUdevProperty("ID_MODEL")) {
            try {
                return $this->getSysfsValue("device/model");
            } catch (\OMV\FileException $e) {
                return "";
            }
        }
        $property = $this->getUdevProperty("ID_MODEL");
        return str_replace("_", " ", $property);
    }

    /**
     * Get the device vendor (via udev or sysfs).
     * @return string The device vendor, otherwise an empty string.
     *   Note, underscores are replaced by blanks when the value is
     *   retrieved via udev.
     */
    public function getVendor()
    {
        if (false === $this->hasUdevProperty("ID_VENDOR")) {
            try {
                return $this->getSysfsValue("device/vendor");
            } catch (\OMV\FileException $e) {
                return "";
            }
        }
        $property = $this->getUdevProperty("ID_VENDOR");
        return str_replace("_", " ", $property);
    }

    /**
     * Get the device serial number.
     * @return string The device serial number, otherwise an empty string.
     *   Note, underscores are replaced by blanks.
     */
    public function getSerialNumber()
    {
        if (true === $this->hasUdevProperty("ID_SERIAL_SHORT")) {
            $serial = $this->getUdevProperty("ID_SERIAL_SHORT");
            return str_replace("_", " ", $serial);
        } elseif (true === $this->hasUdevProperty("ID_SERIAL")) {
            return $this->getUdevProperty("ID_SERIAL");
        }
        return "";
    }

    /**
     * Get the World Wide Name (WWN) identifier.
     * @return string The WWN identifier, otherwise an empty string.
     */
    public function getWorldWideName()
    {
        if (false === $this->hasUdevProperty("ID_WWN")) {
            return "";
        }
        return $this->getUdevProperty("ID_WWN");
    }

    /**
     * See interface definition.
     */
    public function getDescription()
    {
        $model = $this->getModel();
        return sprintf(
            "%s [%s, %s]",
            !empty($model) ? $model : gettext("N/A"),
            $this->getPreferredDeviceFile(),
            binary_format($this->getSize())
        );
    }

    /**
     * Check if the device is of rotational or non-rotational type.
     * See https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt
     * @return boolean TRUE if device is rotational, otherwise FALSE.
     */
    public function isRotational()
    {
        // Use udev property.
        if (true === $this->hasUdevProperty("ID_SSD")) {
            $property = $this->getUdevProperty("ID_SSD");
            // If ID_SSD is not 1 then it is rotational.
            return (0 == strcasecmp("1", $property)) ? false : true;
        }
        if (true === $this->hasUdevProperty("ID_ATA_ROTATION_RATE_RPM")) {
            $property = $this->getUdevProperty("ID_ATA_ROTATION_RATE_RPM");
            // If ID_ATA_ROTATION_RATE_RPM is non-zero then it is rotational.
            return (0 == strcasecmp("0", $property)) ? false : true;
        }
        if (true === $this->hasUdevProperty("ID_ATA_FEATURE_SET_AAM")) {
            $property = $this->getUdevProperty("ID_ATA_FEATURE_SET_AAM");
            // If ID_ATA_FEATURE_SET_AAM is non-zero then it is rotational.
            return (0 == strcasecmp("0", $property)) ? false : true;
        }
        // Use kernel attribute.
        try {
            // See https://www.kernel.org/doc/Documentation/ABI/stable/sysfs-block
            // 0 => SSD, 1 => HDD
            return ("0" != $this->getSysfsValue("queue/rotational"));
        } catch (\OMV\FileException $e) {
            // Fallthrough
        }
        // Use heuristic.
        $model = $this->getModel();
        if (is_string($model) && (false !== strstr($model, "SSD"))) {
            return false;
        }
        return true;
    }

    /**
    * Check if the device is removable.
    * @return boolean TRUE if device is removable, otherwise FALSE.
    */
    public function isRemovable()
    {
        try {
            return ("1" == $this->getSysfsValue("removable"));
        } catch (\OMV\FileException $e) {
            return false;
        }
    }

    /**
     * Check if the given device is a hardware/software RAID device.
     * @return boolean TRUE if the device is a hardware/software RAID,
     *   otherwise FALSE.
     */
    public function isRaid()
    {
        return false;
    }

    /**
     * Check if the given device is an USB device.
     * @return boolean TRUE if the device is connected via USB,
     *   otherwise FALSE.
     */
    public function isUsb()
    {
        // Identify USB devices via 'ID_BUS=usb'.
        if (true === $this->hasUdevProperty("ID_BUS")) {
            $property = $this->getUdevProperty("ID_BUS");
            if (0 == strcasecmp("usb", $property)) {
                return true;
            }
        }
        // Identify USB devices via 'ID_USB_DRIVER=usb-storage'.
        if (true === $this->hasUdevProperty("ID_USB_DRIVER")) {
            $property = $this->getUdevProperty("ID_USB_DRIVER");
            if (0 == strcasecmp("usb-storage", $property)) {
                return true;
            }
        }
        // Identify USB devices via 'ID_DRIVE_THUMB=1'.
        if (true === $this->hasUdevProperty("ID_DRIVE_THUMB")) {
            $property = $this->getUdevProperty("ID_DRIVE_THUMB");
            if (0 == strcasecmp("1", $property)) {
                return true;
            }
        }
        // Identify USB devices via 'ID_PATH=xxx-usb-xxx'.
        // Example:
        // ID_PATH=pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0
        // ID_PATH=pci-0000:00:12.2-usb-0:3:1.0-scsi-0:0:0:0
        if (true === $this->hasUdevProperty("ID_PATH")) {
            $property = $this->getUdevProperty("ID_PATH");
            if (1 == preg_match('/^.+-usb-.+$/i', $property)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Check if the given device is connected via ATA.
     * @return boolean TRUE if the device is connected via ATA,
     *   otherwise FALSE.
     */
    public function isAta()
    {
        if (false === $this->hasUdevProperty("ID_BUS")) {
            return false;
        }
        $property = $this->getUdevProperty("ID_BUS");
        return (0 == strcasecmp("ata", $property)) ? true : false;
    }

    /**
     * Check if the given device is an SD or MMC device.
     * @return boolean TRUE if device is SD or MMC, otherwise FALSE.
     */
    public function isMmc()
    {
        return false;
    }

    /**
     * Check if the given device is read-only.
     * @return boolean TRUE if the device is read-only, otherwise FALSE.
     */
    public function isReadOnly()
    {
        return false;
    }

    /**
     * Check if a medium is available. The method should always return TRUE
     * for devices without removable media.
     * @return boolean Returns FALSE if no medium is available.
     */
    public function IsMediaAvailable()
    {
        return true;
    }

    /**
     * Check if the given device is hot pluggable.
     * @return boolean TRUE if the device is hot pluggable, otherwise FALSE.
     */
    public function isHotPluggable(): bool
    {
        return $this->isUsb();
    }

    /**
     * Check if the given device has S.M.A.R.T. support.
     * @return boolean Returns TRUE if the device supports S.M.A.R.T.,
     *   otherwise FALSE.
     */
    public function hasSmartSupport()
    {
        // Check whether this object implements the SmartInterface interface.
        return is_a($this, "\OMV\System\Storage\SmartInterface");
    }

    /**
     * Assert that the given device has S.M.A.R.T. support.
     * @return void
     * @throw \OMV\AssertException
     */
    public function assertHasSmartSupport()
    {
        if (false === $this->hasSmartSupport()) {
            throw new \OMV\AssertException(
                "Device '%s' does not support S.M.A.R.T.",
                $this->getDeviceFile()
            );
        }
    }

    /**
     * Wipe the storage device.
     * @return void
     */
    public function wipe()
    {
        $cmdArgs = [];
        $cmdArgs[] = "--zap-all";
        $cmdArgs[] = escapeshellarg($this->getDeviceFile());
        $cmd = new \OMV\System\Process("sgdisk", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->execute();
    }

    /**
     * Get the device names of the partitions of this device.
     * @return array Returns an array of device names.
     */
    public function getPartitionDeviceNames()
    {
        $result = [];
        $pattern = sprintf(
            "/sys/block/%s/*/partition",
            $this->getDeviceName()
        );
        foreach (glob($pattern) as $path) {
            $result[] = basename(substr($path, 0, -10));
        }
        return $result;
    }

    /**
     * Enumerate devices matching the given storage device filter.
     * @param int filter Defines the storage device filter bitmask.
     *   Supports OMV_STORAGE_DEVICE_KIND_* and OMV_STORAGE_DEVICE_TRAIT_*
     *   bits. Defaults to OMV_STORAGE_DEVICE_KIND_DEFAULT.
     * @param LogicalOperator traitOperator Controls how TRAIT_* bits are matched.
     *   LogicalOperator::ALL requires all specified traits to be present (AND,
     *   default). LogicalOperator::ANY requires at least one trait (OR).
     * @return array|boolean A list of devicefile names, otherwise FALSE.
     */
    public static function enumerate(
        int $filter = OMV_STORAGE_DEVICE_KIND_DEFAULT,
        \OMV\LogicalOperator $traitOperator = \OMV\LogicalOperator::ALL
    ) {
        $result = [];
        $mngr = Backend\Manager::getInstance();
        foreach ($mngr as $backendk => $backendv) {
            if (!self::matchFilter(
                $filter,
                $backendv->getType(),
                $traitOperator
            )) {
                continue;
            }
            if (false === ($devs = $backendv->enumerate())) {
                return false;
            }
            $result = array_merge($result, $devs);
        }
        $result = array_unique($result);
        if (!sort($result, SORT_NATURAL)) {
            return false;
        }
        return $result;
    }

    /**
     * Enumerate devices matching the given storage device filter and return
     * them as instantiated StorageDeviceInterface objects.
     *
     * This is the object-returning counterpart of enumerate(). Use this method
     * whenever you need to work with the returned devices immediately (e.g. read
     * properties, call methods), as it avoids a second instantiation round-trip
     * that would be required if you called enumerate() and then re-instantiated
     * each device yourself.
     *
     * Example – all non-removable, rotational block devices as objects:
     * <code>
     * $devs = StorageDevice::enumerateObjects(
     *     OMV_STORAGE_DEVICE_KIND_DEFAULT,
     *     \OMV\LogicalOperator::ALL,
     *     function(StorageDeviceInterface $sd): bool {
     *         return !$sd->isRemovable() && $sd->isRotational();
     *     }
     * );
     * </code>
     *
     * @param int $filter Defines the storage device filter bitmask.
     *   Supports OMV_STORAGE_DEVICE_KIND_* and OMV_STORAGE_DEVICE_TRAIT_*
     *   bits. Defaults to OMV_STORAGE_DEVICE_KIND_DEFAULT.
     * @param \OMV\LogicalOperator $traitOperator Controls how TRAIT_* bits are
     *   matched. LogicalOperator::ALL requires all specified traits to be present
     *   (AND, default). LogicalOperator::ANY requires at least one trait (OR).
     * @param callable|null $predicate Optional Stage-2 predicate. Receives a
     *   StorageDeviceInterface instance and must return TRUE to include the device.
     *   Omit or pass NULL to include all instantiated devices.
     * @return StorageDeviceInterface[]|false A list of StorageDeviceInterface
     *   objects sorted by device file name, otherwise FALSE.
     */
    public static function enumerateObjects(
        int $filter = OMV_STORAGE_DEVICE_KIND_DEFAULT,
        \OMV\LogicalOperator $traitOperator = \OMV\LogicalOperator::ALL,
        ?callable $predicate = null
    ) {
        if (false === ($deviceFiles = self::enumerate($filter, $traitOperator))) {
            return false;
        }
        $result = [];
        foreach ($deviceFiles as $deviceFile) {
            $sd = self::getStorageDevice($deviceFile);
            if (is_null($sd)) {
                continue;
            }
            if ($predicate !== null && $predicate($sd) !== true) {
                continue;
            }
            $result[] = $sd;
        }
        return $result;
    }

    /**
     * Enumerate all unused devices matching the given storage device filter.
     * This list contains all matching devices except those that are used by
     * other storage devices
     * (e.g. LVM as physical volume or a member of a Software RAID).
     * @param int filter Defines the storage device filter bitmask.
     *   Supports OMV_STORAGE_DEVICE_KIND_* and OMV_STORAGE_DEVICE_TRAIT_*
     *   bits. Defaults to OMV_STORAGE_DEVICE_KIND_DEFAULT.
     * @param LogicalOperator traitOperator Controls how TRAIT_* bits are matched.
     *   LogicalOperator::ALL requires all specified traits to be present (AND,
     *   default). LogicalOperator::ANY requires at least one trait (OR).
     * @return array|boolean A list of devicefile names, otherwise FALSE.
     */
    public static function enumerateUnused(
        int $filter = OMV_STORAGE_DEVICE_KIND_DEFAULT,
        \OMV\LogicalOperator $traitOperator = \OMV\LogicalOperator::ALL
    ) {
        // Append all available storage devices.
        if (false === ($result = self::enumerate($filter, $traitOperator))) {
            return false;
        }
        // Remove used devices.
        $mngr = Backend\Manager::getInstance();
        foreach ($mngr as $backendk => $backendv) {
            // Always collect and remove all slave devices.
            //			if (!($type & $backendv->getType()))
            //				continue;
            if (false === ($slaves = $backendv->enumerateSlaves())) {
                return false;
            }
            if (empty($slaves)) {
                continue;
            }
            $result = array_diff($result, $slaves);
        }
        $result = array_unique($result);
        if (!sort($result, SORT_NATURAL)) {
            return false;
        }
        return $result;
    }

    /**
     * Enumerate all unused devices matching the given storage device filter
     * and return them as instantiated StorageDeviceInterface objects.
     *
     * This is the object-returning counterpart of enumerateUnused(). The result
     * contains all matching devices except those that are consumed by other
     * storage devices (e.g. LVM physical volumes or Software RAID members).
     *
     * @param int $filter Defines the storage device filter bitmask.
     *   Supports OMV_STORAGE_DEVICE_KIND_* and OMV_STORAGE_DEVICE_TRAIT_*
     *   bits. Defaults to OMV_STORAGE_DEVICE_KIND_DEFAULT.
     * @param \OMV\LogicalOperator $traitOperator Controls how TRAIT_* bits are
     *   matched. LogicalOperator::ALL requires all specified traits to be present
     *   (AND, default). LogicalOperator::ANY requires at least one trait (OR).
     * @param callable|null $predicate Optional Stage-2 predicate. Receives a
     *   StorageDeviceInterface instance and must return TRUE to include the device.
     *   Omit or pass NULL to include all instantiated devices.
     * @return StorageDeviceInterface[]|false A list of StorageDeviceInterface
     *   objects sorted by device file name, otherwise FALSE.
     */
    public static function enumerateUnusedObjects(
        int $filter = OMV_STORAGE_DEVICE_KIND_DEFAULT,
        \OMV\LogicalOperator $traitOperator = \OMV\LogicalOperator::ALL,
        ?callable $predicate = null
    ) {
        if (false === ($deviceFiles = self::enumerateUnused(
            $filter,
            $traitOperator
        ))) {
            return false;
        }
        $result = [];
        foreach ($deviceFiles as $deviceFile) {
            $sd = self::getStorageDevice($deviceFile);
            if (is_null($sd)) {
                continue;
            }
            if ($predicate !== null && $predicate($sd) !== true) {
                continue;
            }
            $result[] = $sd;
        }
        return $result;
    }

    /**
     * Enumerate all used devices matching the given storage device filter.
     * The list contains all those devices that are used by the matching
     * storage devices, e.g. all members of a LVM or MD RAID.
     * @param int filter Defines the storage device filter bitmask.
     *   Supports OMV_STORAGE_DEVICE_KIND_* and OMV_STORAGE_DEVICE_TRAIT_*
     *   bits. Defaults to OMV_STORAGE_DEVICE_KIND_DEFAULT.
     * @param LogicalOperator traitOperator Controls how TRAIT_* bits are matched.
     *   LogicalOperator::ALL requires all specified traits to be present (AND,
     *   default). LogicalOperator::ANY requires at least one trait (OR).
     * @return array|boolean A list of devicefile names, otherwise FALSE.
     */
    public static function enumerateUsed(
        int $filter = OMV_STORAGE_DEVICE_KIND_DEFAULT,
        \OMV\LogicalOperator $traitOperator = \OMV\LogicalOperator::ALL
    ) {
        $result = [];
        $mngr = Backend\Manager::getInstance();
        foreach ($mngr as $backendk => $backendv) {
            if (!self::matchFilter($filter, $backendv->getType(), $traitOperator)) {
                continue;
            }
            if (false === ($slaves = $backendv->enumerateSlaves())) {
                return false;
            }
            if (empty($slaves)) {
                continue;
            }
            $result = array_merge($result, $slaves);
        }
        $result = array_unique($result);
        if (!sort($result, SORT_NATURAL)) {
            return false;
        }
        return $result;
    }

    /**
     * Enumerate all used devices matching the given storage device filter
     * and return them as instantiated StorageDeviceInterface objects.
     *
     * This is the object-returning counterpart of enumerateUsed(). The result
     * contains those devices that are consumed by the matching storage backends
     * (e.g. all members of an MD RAID or LVM volume group).
     *
     * @param int $filter Defines the storage device filter bitmask.
     *   Supports OMV_STORAGE_DEVICE_KIND_* and OMV_STORAGE_DEVICE_TRAIT_*
     *   bits. Defaults to OMV_STORAGE_DEVICE_KIND_DEFAULT.
     * @param \OMV\LogicalOperator $traitOperator Controls how TRAIT_* bits are
     *   matched. LogicalOperator::ALL requires all specified traits to be present
     *   (AND, default). LogicalOperator::ANY requires at least one trait (OR).
     * @param callable|null $predicate Optional Stage-2 predicate. Receives a
     *   StorageDeviceInterface instance and must return TRUE to include the device.
     *   Omit or pass NULL to include all instantiated devices.
     * @return StorageDeviceInterface[]|false A list of StorageDeviceInterface
     *   objects sorted by device file name, otherwise FALSE.
     */
    public static function enumerateUsedObjects(
        int $filter = OMV_STORAGE_DEVICE_KIND_DEFAULT,
        \OMV\LogicalOperator $traitOperator = \OMV\LogicalOperator::ALL,
        ?callable $predicate = null
    ) {
        if (false === ($deviceFiles = self::enumerateUsed(
            $filter,
            $traitOperator
        ))) {
            return false;
        }
        $result = [];
        foreach ($deviceFiles as $deviceFile) {
            $sd = self::getStorageDevice($deviceFile);
            if (is_null($sd)) {
                continue;
            }
            if ($predicate !== null && $predicate($sd) !== true) {
                continue;
            }
            $result[] = $sd;
        }
        return $result;
    }

    /**
     * Check if the given device (and it's canonical device file) is
     * used/consumed by another storage device matching the given filter.
     * @param string deviceFile Specifies the device file, e.g.
     *   <ul>
     *   \li /dev/sdb
     *   \li /dev/md1
     *   \li /dev/cciss/c0d0
     *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
     *   \li /dev/disk/by-label/DATA
     *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
     *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
     *   </ul>
     * @param int filter Defines the storage device filter bitmask.
     *   Supports OMV_STORAGE_DEVICE_KIND_* and OMV_STORAGE_DEVICE_TRAIT_*
     *   bits. Defaults to OMV_STORAGE_DEVICE_KIND_DEFAULT.
     * @param LogicalOperator traitOperator Controls how TRAIT_* bits are matched.
     *   LogicalOperator::ALL requires all specified traits to be present (AND,
     *   default). LogicalOperator::ANY requires at least one trait (OR).
     * @return boolean TRUE if the given device is used/consumed by another
     *   storage device, otherwise FALSE.
     */
    public static function isUsed(
        $deviceFile,
        int $filter = OMV_STORAGE_DEVICE_KIND_DEFAULT,
        \OMV\LogicalOperator $traitOperator = \OMV\LogicalOperator::ALL
    ) {
        $result = false;
        $mngr = Backend\Manager::getInstance();
        $canonicalDeviceFile = realpath($deviceFile);
        foreach ($mngr as $backendk => $backendv) {
            if (!self::matchFilter($filter, $backendv->getType(), $traitOperator)) {
                continue;
            }
            if (false === ($slaves = $backendv->enumerateSlaves())) {
                return false;
            }
            // Search with the canonical device file first as most backends
            // return a list of these device files.
            if (false !== $canonicalDeviceFile) {
                if (in_array($canonicalDeviceFile, $slaves)) {
                    $result = true;
                    break;
                }
            }
            // Finally use the specified device file to complete the search.
            if ($deviceFile !== $canonicalDeviceFile) {
                if (in_array($deviceFile, $slaves)) {
                    $result = true;
                    break;
                }
            }
        }
        return $result;
    }

    /**
     * Factory method to get the object of the class which implements the
     * given storage device.
     * @param string deviceFile Specifies the device file, e.g.
     *   <ul>
     *   \li /dev/sdb
     *   \li /dev/md1
     *   \li /dev/cciss/c0d0
     *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
     *   \li /dev/disk/by-label/DATA
     *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
     *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
     *   </ul>
     * @return object|null The object of the class implementing the given
     *   storage device, otherwise NULL.
     */
    public static function getStorageDevice($deviceFile)
    {
        $mngr = Backend\Manager::getInstance();
        if (null == ($backend = $mngr->getBackend($deviceFile))) {
            return null;
        }
        $result = $backend->getImpl($deviceFile);
        if (is_null($result) || !$result->exists()) {
            return null;
        }
        return $result;
    }

    /**
     * @see StorageDevice::getStorageDevice()
     * @return object The object of the class implementing the given
     *   storage device.
     * @throw \OMV\AssertException
     */
    public static function assertGetStorageDevice($deviceFile)
    {
        $result = self::getStorageDevice($deviceFile);
        if (is_null($result) || !$result->exists()) {
            throw new \OMV\AssertException(
                "Device '%s' does not exist.",
                $deviceFile
            );
        }
        return $result;
    }

    /**
     * Normalize deprecated TYPE_* filter bits to the new KIND_* bits.
     */
    private static function normalizeFilter(int $filter): int
    {
        if (0 !== ($filter & OMV_STORAGE_DEVICE_TYPE_DISK)) {
            $filter |= OMV_STORAGE_DEVICE_KIND_GENERIC;
        }
        if (0 !== ($filter & OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID)) {
            $filter |= OMV_STORAGE_DEVICE_KIND_MD;
        }
        if (0 !== ($filter & OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE)) {
            $filter |= OMV_STORAGE_DEVICE_KIND_LOOP;
        }
        if (0 !== ($filter & OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER)) {
            $filter |= OMV_STORAGE_DEVICE_KIND_DM;
        }
        if (0 !== ($filter & OMV_STORAGE_DEVICE_TYPE_SCSIGENERIC)) {
            $filter |= OMV_STORAGE_DEVICE_KIND_SG;
        }
        return $filter;
    }

    /**
     * Check if the given backend type matches the specified filter.
     * KIND bits are matched with OR semantics. TRAIT bits are matched
     * according to $traitOperator (ALL = AND semantics, ANY = OR semantics).
     */
    private static function matchFilter(
        int $filter,
        int $backendType,
        \OMV\LogicalOperator $traitOperator = \OMV\LogicalOperator::ALL
    ): bool {
        $filter = self::normalizeFilter($filter);
        $kindMask = $filter & OMV_STORAGE_DEVICE_KIND_ALL;
        $traitMask = $filter & OMV_STORAGE_DEVICE_TRAIT_ALL;
        if ((0 !== $kindMask) && (0 === ($backendType & $kindMask))) {
            return false;
        }
        if (0 !== $traitMask) {
            if ($traitOperator === \OMV\LogicalOperator::ALL) {
                if ($traitMask !== ($backendType & $traitMask)) {
                    return false;
                }
            } else {
                if (0 === ($backendType & $traitMask)) {
                    return false;
                }
            }
        }
        return (0 !== ($kindMask | $traitMask));
    }
}
