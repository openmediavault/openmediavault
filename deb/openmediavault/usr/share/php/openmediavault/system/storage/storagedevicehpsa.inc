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

/**
 * This class provides a simple interface to handle HP Smart Array RAID
 * controllers using the hpsa driver.
 * @ingroup api
 */
class StorageDeviceHPSA extends StorageDeviceSD
{
    public function isRaid()
    {
        return "RAID" == substr($this->getRaidLevel(), 0, 4);
    }

    /**
     * Get the RAID level of the device. If it is a logical device,
     * then 'N/A' will be returned, otherwise 'RAID 0', 'RAID 1(+0)'
     * or 'RAID <N>'.
     *
     * @link https://www.kernel.org/doc/html/latest/scsi/hpsa.html#hpsa-specific-disk-attributes
     * @link https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L654
     * @link https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L694
     * @link https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L702
     *
     * @return string Returns the RAID level.
     */
    public function getRaidLevel(): string
    {
        $hctl = $this->getHCTL();
        $filePath = sprintf(
            "/sys/class/scsi_disk/%d:%d:%d:%d/device/raid_level",
            $hctl['host'],
            $hctl['channel'],
            $hctl['target'],
            $hctl['lun']
        );
        if (!file_exists($filePath)) {
            return "N/A";
        }
        return trim(file_get_contents($filePath));
    }
}
