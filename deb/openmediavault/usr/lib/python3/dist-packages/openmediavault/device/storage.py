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
from cached_property import cached_property

import abc
import importlib
import os
import pkgutil
import re

import openmediavault.string

from .block import BlockDevice


class StorageDevice(BlockDevice):
    @property
    def parent(self):
        """
        Create a new instance from the parent device file.
        Examples:
        - /dev/sdb1 => /dev/sdb
        - /dev/cciss/c0d0p2 => /dev/cciss/c0d0
        - /dev/mapper/vg0-lv0 => None
        - /dev/dm-0 => None
        - /dev/md0 => None
        - /dev/loop0 => None
        - /dev/vdc2 => /dev/vdc
        - /dev/nvme0n1p1 -> /dev/nvme0n1
        :return: Returns a :class:`StorageDevice` if a parent device
            exists, otherwise ``None``.
        :rtype: :class:`StorageDevice`|None
        """
        return None

    @property
    def description(self):
        """
        Get the description of the device.
        :return: The device description.
        :rtype: str
        """
        return ''

    @property
    def model(self):
        """
        Get the device model.
        :return: The device model, otherwise an empty string.
        :rtype: str
        """
        return openmediavault.string.unescape_blank(
            self.udev_property('ID_MODEL_ENC', '')
        )

    @property
    def vendor(self):
        """
        Get the device vendor.
        :return: The device vendor, otherwise an empty string.
        :rtype: str
        """
        return openmediavault.string.unescape_blank(
            self.udev_property('ID_VENDOR_ENC', '')
        )

    @property
    def serial(self):
        """
        Get the device serial number.
        :return: The device serial number, otherwise an empty string.
        :rtype: str
        """
        serial = self.udev_property('ID_SERIAL_SHORT', '')
        return serial.replace('_', ' ')

    @property
    def is_rotational(self):
        """
        Check if the device is of rotational or non-rotational type.
        See https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt
        :return: Return True if device is rotational, otherwise False.
        :rtype: bool
        """
        if self.has_udev_property('ID_SSD'):
            # If ID_SSD is not 1 then it is rotational.
            return self.udev_property('ID_SSD') != '1'
        if self.has_udev_property('ID_ATA_ROTATION_RATE_RPM'):
            # If ID_ATA_ROTATION_RATE_RPM is non-zero then it is rotational.
            return self.udev_property('ID_ATA_ROTATION_RATE_RPM') != '0'
        if self.has_udev_property('ID_ATA_FEATURE_SET_AAM'):
            # If ID_ATA_FEATURE_SET_AAM is non-zero then it is rotational.
            return self.udev_property('ID_ATA_FEATURE_SET_AAM') != '0'
        # Use kernel attribute.
        file = '/sys/block/{}/queue/rotational'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                # 0 => SSD, 1 => HDD
                return f.readline().strip() != '0'
        except (IOError, FileNotFoundError):
            pass
        # Use heuristic.
        return 'SSD' not in self.model

    @property
    def is_removable(self):
        """
        Check if the device is removable.
        :return: Returns ``True`` if the device is removable,
            otherwise ``False``.
        :rtype: bool
        """
        file = '/sys/block/{}/removable'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                return f.readline().strip() == '1'
        except (IOError, FileNotFoundError):
            pass
        return False

    @property
    def is_usb(self):
        """
        Check if the given device is an USB device.
        :return: Returns ``True`` if the device is connected via USB,
            otherwise ``False``.
        :rtype: bool
        """
        # Identify USB devices via 'ID_BUS=usb'.
        if self.has_udev_property('ID_BUS'):
            return self.udev_property('ID_BUS') == 'usb'
        # Identify USB devices via 'ID_USB_DRIVER=usb-storage'.
        if self.has_udev_property('ID_USB_DRIVER'):
            return self.udev_property('ID_USB_DRIVER') == 'usb-storage'
        # Identify USB devices via 'ID_DRIVE_THUMB=1'.
        if self.has_udev_property('ID_DRIVE_THUMB'):
            return self.udev_property('ID_DRIVE_THUMB') == '1'
        # Identify USB devices via 'ID_PATH=xxx-usb-xxx'.
        # Example:
        # ID_PATH=pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0
        # ID_PATH=pci-0000:00:12.2-usb-0:3:1.0-scsi-0:0:0:0
        if self.has_udev_property('ID_PATH'):
            value = self.udev_property('ID_PATH')
            return re.match(r'^.+-usb-.+$', value)
        return False

    @property
    def is_read_only(self):
        """
        Check if the given device is read-only.
        :return: Returns ``True`` if the device is read-only,
            otherwise ``False``.
        :rtype: bool
        """
        return False

    @property
    def is_media_available(self):
        """
        Check if a medium is available.
        :return: Returns ``True`` if the medium is available,
            otherwise ``False``.
        :rtype: bool
        """
        return True

    @property
    def is_raid(self):
        """
        Check if the given device is a hardware/software RAID device.
        :return: Returns ``True`` if the device is a hardware/software
            RAID, otherwise ``False``.
        :rtype: bool
        """
        return False

    @property
    def has_smart_support(self):
        """
        Check if the given device has S.M.A.R.T. support.
        :return: Returns ``True`` if the device supports S.M.A.R.T.,
            otherwise ``False``.
        :rtype: bool
        """
        return True

    @property
    def smart_device_type(self):
        """
        Identify the device type required by the smartctl utility
        program.
        @see https://www.smartmontools.org/browser/trunk/smartmontools/smartctl.8.in
        :return: Returns the identified device type, e.g. 'sat',
            'scsi', 'usbcypress', ... or an empty string.
        :rtype: str
        """
        return ''

    def wipe(self):
        """
        Wipe the storage device.
        """
        _ = openmediavault.subprocess.check_output(
            ['sgdisk', '--zap-all', self.device_file]
        )

    @classmethod
    def from_device_file(cls, device_file):
        """
        Create a new instance from the given device file.
        :param device_file: The canonical path of a device file.
        :type device_file: str
        :return: Returns a :class:`StorageDevice`.
        :rtype: :class:`StorageDevice`
        """
        # Load the plugins.
        plugins = []
        plugins_dir = os.path.join(os.path.dirname(__file__), 'plugins')
        modules = [name for _, name, _ in pkgutil.iter_modules([plugins_dir])]
        for module_name in modules:
            mod = importlib.import_module(
                '..plugins.{}'.format(module_name), __name__
            )
            try:
                plugin_inst = mod.StorageDevicePlugin()
                if not isinstance(plugin_inst, IStorageDevicePlugin):
                    continue
                plugins.append(plugin_inst)
            except AttributeError:
                pass
        # Check if a plugin implements the `StorageDevice` class
        # for the given device file.
        for plugin_inst in plugins:
            if plugin_inst.match(device_file):
                return plugin_inst.from_device_file(device_file)
        # If no plugin is responsible, then return an instance of
        # the default class.
        return StorageDevice(device_file)

    @cached_property
    def host_driver(self):
        """
        Get the driver name of the host device this storage device is
        connected to, e.g. 'hpsa', 'arcmsr' or 'ahci'.
        :return: Returns the driver name of the host device or None.
        :rtype: str | None
        """
        # Try to get the driver via 'driver'.
        host_path = '/sys/block/{}/device/../..'.format(self.device_name(True))
        driver_path = os.path.realpath('{}/../driver'.format(host_path))
        if os.path.exists(driver_path):
            return os.path.basename(driver_path)
        # Try to get the driver via 'proc_name'.
        # 'proc_name' is the "name of proc directory" of a driver, if
        # the driver maintained one.
        host_name = None
        real_device_path = os.path.realpath('/sys/block/{}'.format(
            self.device_name(True)))
        for part in real_device_path.split('/'):
            if re.match(r'^host\d+$', part):
                host_name = part
                break
        if host_name:
            try:
                with open(
                        '/sys/class/scsi_host/{}/proc_name'.format(host_name),
                        'r') as f:
                    return f.readline().strip()
            except (IOError, FileNotFoundError):
                pass
        return None


class IStorageDevicePlugin(metaclass=abc.ABCMeta):
    @abc.abstractmethod
    def match(self, device_file):
        """
        Check whether the given device is implemented by
        this class.
        :param device_file: The path of a device file, e.g.
            ``'/dev/sdb'``, ``'/dev/cciss/c0d0'`` or
            ``'/dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0'``.
        :type device_file: str
        :return: Returns ``True`` if this class implements the
            given device file, otherwise ``False``.
        :rtype: bool
        """

    @abc.abstractmethod
    def from_device_file(self, device_file):
        """
        Create a new instance from the given device file.
        :param device_file: The path of a device file, e.g.
            ``'/dev/sdb'``, ``'/dev/cciss/c0d0'`` or
            ``'/dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0'``.
        :type device_file: str
        :return: Returns a :class:`StorageDevice`.
        :rtype: :class:`StorageDevice`
        """
