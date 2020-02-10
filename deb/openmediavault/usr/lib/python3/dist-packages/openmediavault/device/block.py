# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
import os
import pyudev
import re

import openmediavault.subprocess

from .utils import (
    is_block_device,
    is_device_file_by_id,
    is_device_file_by_path,
    is_device_file_by_uuid,
)


class BlockDevice:
    _device_file = None

    def __init__(self, device_file):
        self._device_file = device_file

    @property
    def device_file(self):
        return self._device_file

    @property
    def canonical_device_file(self):
        """
        Get the canonical device file, e.g.

        * /dev/root -> /dev/sde1
        * /dev/disk/by-uuid/4B04EA317E4AA567 -> /dev/sdd1
        * /dev/mapper/vg0-lv0 -> /dev/dm-0

        :return: Return the canonical device file.
        :rtype: str
        """
        return os.path.realpath(self.device_file)

    @property
    def exists(self):
        return is_block_device(self.device_file)

    @property
    def device_links(self):
        """
        Get all device file symlinks, e.g.

        * /dev/disk/by-id/wwn-0x5000cca211cc703c
        * /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
        * /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
        * /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
        * /dev/disk/by-id/ata-WDC_WD15EARS-00MVWB0_WD-WMAZB2574325-part1
        * /dev/disk/by-uuid/fc3e1da5-fd8d-4fda-341e-d0135efa7a7c

        :return: Returns an array of strings with the device files.
        :rtype: list
        """
        context = pyudev.Context()
        device = pyudev.Devices.from_device_file(context, self.device_file)
        return [device_link for device_link in device.device_links]

    @property
    def predictable_device_file(self):
        """
        Get a predictable device file in the following order:

        * /dev/disk/by-id/xxx
        * /dev/disk/by-path/xxx
        * /dev/xxx

        :return: Returns a device file.
        :rtype: str
        """
        if self.has_device_file_by_id():
            return self.device_file_by_id
        if self.has_device_file_by_path():
            return self.device_file_by_path
        return self.canonical_device_file

    def has_device_file_by_id(self):
        """
        Check whether the device has a /dev/disk/by-id/xxx device path.
        :return: Returns TRUE if a disk/by-id device path exists,
            otherwise FALSE.
        :rtype: bool
        """
        return is_device_file_by_id(self.device_file_by_id)

    @property
    def device_file_by_id(self):
        """
        Get the device file, e.g.

        * /dev/disk/by-id/wwn-0x5000cca211cc703c
        * /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
        * /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML-part2

        :return: Returns the device file (/dev/disk/by-id/xxx) if available,
            otherwise None.
        :rtype: str|None
        """
        for device_link in self.device_links:
            if is_device_file_by_id(device_link):
                return device_link
        return None

    def has_device_file_by_path(self):
        """
        Check whether the device has a /dev/disk/by-path/xxx device path.
        :return: Returns TRUE if a disk/by-path device path exists,
            otherwise FALSE.
        :rtype: bool
        """
        return is_device_file_by_path(self.device_file_by_path)

    @property
    def device_file_by_path(self):
        """
        Get the device file, e.g.

        * /dev/disk/by-path/pci-0000:00:17.0-ata-3
        * /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
        * /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:1:0-part1

        :return: Returns the device file (/dev/disk/by-path/xxx) if available,
            otherwise None.
        :rtype: str|None
        """
        for device_link in self.device_links:
            if is_device_file_by_path(device_link):
                return device_link
        return None

    def has_device_file_by_uuid(self):
        """
        Check whether the device has a /dev/disk/by-uuid/xxx device path.
        :return: Returns TRUE if a disk/by-uuid device path exists,
            otherwise FALSE.
        :rtype: bool
        """
        return is_device_file_by_uuid(self.device_file_by_uuid)

    @property
    def device_file_by_uuid(self):
        """
        Get the device file, e.g.

        * /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
        * /dev/disk/by-uuid/2ED43920D438EC29 (NTFS)
        * /dev/disk/by-uuid/7A48-BA97 (FAT)

        :return: Returns the device file (/dev/disk/by-uuid/xxx) if available,
            otherwise None.
        :rtype: str|None
        """
        for device_link in self.device_links:
            if is_device_file_by_uuid(device_link):
                return device_link
        return None

    def device_name(self, canonical=False):
        """
        Get the device name, e.g. sda or hdb.
        :param canonical: If set to True the canonical device file will
            be used. Defaults to False.
        :type canonical: bool
        :return: The device name.
        :rtype: str
        """
        return re.sub(
            r'^/dev/',
            '',
            self.device_file if not canonical else self.canonical_device_file
        )

    @property
    def device_number(self):
        """
        The device number of the associated device as integer.
        See `<https://www.kernel.org/doc/Documentation/devices.txt> for more information`_.
        :rtype: int
        """
        context = pyudev.Context()
        device = pyudev.Devices.from_device_file(context, self.device_file)
        return device.device_number

    @property
    def major_device_number(self):
        """
        Get the major device number.
        See `<https://www.kernel.org/doc/Documentation/devices.txt> for more information`_.
        :return: Returns the major device number.
        :rtype: int
        """
        return os.major(self.device_number)

    @property
    def minor_device_number(self):
        """
        Get the minor device number.
        See `<https://www.kernel.org/doc/Documentation/devices.txt> for more information`_.
        :return: Returns the minor device number.
        :rtype: int
        """
        return os.minor(self.device_number)

    def block_size(self):
        """
        Get the block size.
        :return: Returns the block size.
        :rtype: int
        """
        output = openmediavault.subprocess.check_output(
            ['blockdev', '--getbsz', self.device_file]
        )
        return int(output.decode().strip())

    def size(self):
        """
        Get the device size in bytes.
        :return: Returns the device size in bytes.
        :rtype: int
        """
        output = openmediavault.subprocess.check_output(
            ['blockdev', '--getsize64', self.device_file]
        )
        return int(output.decode().strip())

    @property
    def udev_properties(self):
        """
        Get the udev properties.

        UDEV_LOG=3
        DEVPATH=/devices/pci0000:00/0000:00:10.0/host2/target2:0:1/2:0:1:0/block/sdb
        MAJOR=8
        MINOR=16
        DEVNAME=/dev/sdb
        DEVTYPE=disk
        SUBSYSTEM=block
        ID_SCSI=1
        ID_VENDOR=VMware_
        ID_VENDOR_ENC=VMware\x2c\x20
        ID_MODEL=VMware_Virtual_S
        ID_MODEL_ENC=VMware\x20Virtual\x20S

        DEVLINKS=/dev/disk/by-id/ata-IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-id/scsi-SATA_IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
        DEVNAME=/dev/sda
        DEVPATH=/devices/pci0000:00/0000:00:02.5/host0/target0:0:0/0:0:0:0/block/sda
        DEVTYPE=disk
        ID_ATA=1
        ID_ATA_FEATURE_SET_HPA=1
        ID_ATA_FEATURE_SET_HPA_ENABLED=1
        ID_ATA_FEATURE_SET_PM=1
        ID_ATA_FEATURE_SET_PM_ENABLED=1
        ID_ATA_FEATURE_SET_SECURITY=1
        ID_ATA_FEATURE_SET_SECURITY_ENABLED=0
        ID_ATA_FEATURE_SET_SECURITY_ERASE_UNIT_MIN=22
        ID_ATA_FEATURE_SET_SMART=1
        ID_ATA_FEATURE_SET_SMART_ENABLED=0
        ID_ATA_WRITE_CACHE=1
        ID_ATA_WRITE_CACHE_ENABLED=1
        ID_BUS=ata
        ID_MODEL=IBM-DTTA-351010
        ID_MODEL_ENC=IBM-DTTA-351010\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20
        ID_PART_TABLE_TYPE=dos
        ID_PATH=pci-0000:00:02.5-scsi-0:0:0:0
        ID_PATH_TAG=pci-0000_00_02_5-scsi-0_0_0_0
        ID_REVISION=T56OA7A3
        ID_SCSI_COMPAT=SATA_IBM-DTTA-341050_WF0WABH1579
        ID_SERIAL=IBM-DTTA-341050_WF0WABH1579
        ID_SERIAL_SHORT=WF0WFJH1486
        ID_TYPE=disk
        MAJOR=8
        MINOR=0
        SUBSYSTEM=block
        UDEV_LOG=3
        USEC_INITIALIZED=16872806

        :return: The udev properties.
        :rtype: dict
        """
        context = pyudev.Context()
        device = pyudev.Devices.from_device_file(context, self.device_file)
        result = {}
        for name in device.properties:
            result[name] = device.properties[name]
        return result

    def has_udev_property(self, name):
        """
        Checks if a udev property exists.
        :param name: The name of the property, e.g. ID_VENDOR, ID_MODEL
            or ID_SERIAL_SHORT.
        :type name: str
        :return: Returns True if the property exists, otherwise False.
        :rtype: bool
        """
        return name in self.udev_properties

    def udev_property(self, name, default=None):
        """
        Get the specified udev property.
        :param name: The name of the property, e.g. ID_VENDOR, ID_MODEL
            or ID_SERIAL_SHORT.
        :type name: str
        :param default: The default value if the property does not exist.
            Defaults to None.
        :type default: str
        :return: Returns the requested property.
        :rtype: str
        :raises: :exc:`~exceptions.KeyError`, if the given property is
            not defined and no default value is specified.
        """
        try:
            return self.udev_properties[name]
        except KeyError as e:
            if default is not None:
                return default
            raise e
