# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2018 Volker Theile
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
__all__ = [
    "BlockDevice", "is_block_device", "is_device_file", "is_device_file_by",
    "is_device_file_by_uuid", "is_device_file_by_id",
    "is_device_file_by_label", "is_device_file_by_path"
]

import os
import re
import stat
import openmediavault.subprocess


def is_block_device(path):
    """
    Check if path is a device file, e.g. /dev/sda1.
    :param path: The path to check.
    :return: Return True if path describes a device file,
        otherwise False.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    if not os.path.exists(path):
        return False
    return stat.S_ISBLK(os.stat(path).st_mode)


def is_device_file(path):
    """
    Check if path describes a device file, e.g. /dev/sda1.
    :param path: The path to check.
    :return: Return True if path describes a device file,
        otherwise False.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return True if re.match(r'^\/dev\/.+$', path) else False


def is_device_file_by(path):
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
    * /dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1
    * /dev/disk/by-label/data

    :param path: The path to check.
    :return: Return True if path describes a device file,
        otherwise False.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return True if re.match(r'^\/dev\/disk\/by-\S+\/.+$', path) else False


def is_device_file_by_uuid(path):
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
    * /dev/disk/by-uuid/2ED43920D438EC29 (NTFS)

    :param path: The path to check.
    :return: Return True if path describes a device file,
        otherwise False.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return True if re.match(r'^\/dev\/disk\/by-uuid\/.+$', path) else False


def is_device_file_by_id(path):
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1
    * /dev/disk/by-id/wwn-0x4002c554a4d79cb9-part2

    :param path: The path to check.
    :return: Return True if path describes a device file,
        otherwise False.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return True if re.match(r'^\/dev\/disk\/by-id\/.+$', path) else False


def is_device_file_by_label(path):
    """
    Check if path describes a device file, e.g. /dev/disk/by-label/data
    :param path: The path to check.
    :return: Return True if path describes a device file,
        otherwise False.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return True if re.match(r'^\/dev\/disk\/by-label\/.+$', path) else False


def is_device_file_by_path(path):
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-path/pci-0000:00:17.0-ata-3
    * /dev/disk/by-path/pci-0000:00:17.0-ata-3-part2

    :param path: The path to check.
    :return: Return True if path describes a device file,
        otherwise False.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return True if re.match(r'^\/dev\/disk\/by-path\/.+$', path) else False


class BlockDevice:
    _path = None
    _udev_properties = {}

    def __init__(self, path):
        self._path = path

    @property
    def exists(self):
        return is_block_device(self.device_file)

    @property
    def device_file(self):
        return self._path

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

    def get_device_file_symlinks(self):
        """
        Get all device file symlinks via udev, e.g.

        * /dev/disk/by-id/wwn-0x5000cca211cc703c
    	* /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
    	* /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
    	* /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
    	* /dev/disk/by-id/ata-WDC_WD15EARS-00MVWB0_WD-WMAZB2574325-part1
    	* /dev/disk/by-uuid/fc3e1da5-fd8d-4fda-341e-d0135efa7a7c

        :return: Returns an array of strings with the device files.
        :rtype: list
        """
        result = []
        if not self.has_udev_property('DEVLINKS'):
            return result
        dev_links = self.get_udev_property('DEVLINKS').split(' ')
        for dev_link in dev_links:
            dev_link = dev_link.strip()
            if is_device_file(dev_link):
                result.append(dev_link)
            else:
                result.append('/dev/{}'.format(dev_link))
        return result

    def device_name(self, canonical=False):
        """
        Get the device name, e.g. sda or hdb.
        :param canonical: If set to True the canonical device file will
   	        be used. Defaults to False.
        :type canonical: bool
        :return: The device name.
        :rtype: str
        """
        device_file = self.device_file if not canonical \
            else self.canonical_device_file
        return device_file.replace('/dev/', '')

    @property
    def udev_properties(self):
        """
        Get the udev properties.
        :return: The udev properties.
        :rtype: dict
        """
        return self._udev_properties

    def has_udev_property(self, prop_id):
        """
        Checks if a udev property exists.
        :param prop_id: The name of the property, e.g. ID_VENDOR, ID_MODEL
            or ID_SERIAL_SHORT.
        :type prop_id: str
        :return: Returns True if the property exists, otherwise False.
        :rtype: bool
        """
        self.query_udev_properties()
        return prop_id in self.udev_properties

    def get_udev_property(self, prop_id):
        """
        Get the specified udev property.
        :param prop_id: The name of the property, e.g. ID_VENDOR, ID_MODEL
            or ID_SERIAL_SHORT.
        :type prop_id: str
        :return: Returns the requested property, otherwise None.
        :rtype: None|str
        """
        if not self.has_udev_property(prop_id):
            return None
        return self.udev_properties[prop_id]

    def query_udev_properties(self, force=False):
        """
	    Queries the udev database for device information stored in the
	    udev database.
	    :param force: Force the collection of the information, although
            the information is already cached. Defaults to False.
        :type force: bool
        :return: Return True if the properties has been queried.
        :rtype: bool
        """
        if not force and self._udev_properties:
            return False
        output = openmediavault.subprocess.check_output([
            'udevadm', 'info', '--query', 'property', '--name',
            self.device_file
        ])
        """
        Parse output:
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
        """
        self._udev_properties = {}
        for line in output.splitlines():
            parts = line.decode().split("=")
            if len(parts) != 2:
                continue
            # Strip only the key, do not touch the value.
            self._udev_properties[parts[0].strip()] = parts[1]
        return True
