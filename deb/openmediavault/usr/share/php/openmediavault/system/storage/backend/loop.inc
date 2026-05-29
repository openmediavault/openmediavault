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

namespace OMV\System\Storage\Backend;

/**
 * Implements the storage device backend for Linux loop devices.
 * @ingroup api
 */
class Loop extends BackendAbstract
{
    protected $regex = "loop[0-9]+";

    public function getType()
    {
        return OMV_STORAGE_DEVICE_KIND_LOOP |
            OMV_STORAGE_DEVICE_TRAIT_BLOCK |
            OMV_STORAGE_DEVICE_TRAIT_VIRTUAL |
            OMV_STORAGE_DEVICE_TRAIT_STACKED;
    }

    public function enumerate()
    {
        return $this->enumerateProcFs($this->regex);
    }

    public function isTypeOf($deviceFile)
    {
        return $this->isTypeOfByName($deviceFile, $this->regex);
    }

    public function getImpl($args)
    {
        return new \OMV\System\Storage\StorageDeviceLoop($args);
    }

    public function fsDeviceFile($deviceFile, $partition = 1)
    {
        return $deviceFile;
    }
}
