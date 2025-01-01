# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
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
import re
from typing import NamedTuple, Optional

import openmediavault.device
from cached_property import cached_property


class HCTL(NamedTuple):
    """
    The SCSI address of a device.

    The following fields are included:
    - HBA number [host]
    - Channel on the HBA [channel]
    - SCSI target ID [target]
    - LUN [lun]
    """
    host: int
    channel: int
    target: int
    lun: int

    @classmethod
    def from_dev_path(cls, dev_path: str):
        """
        Get SCSI address from a given devpath.

        Example:
        /devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0/block/sda

        :param dev_path: The devpath to process.
        :return: The SCSI address of the given devpath.
        """
        # The path '/sys/<DEVPATH>/device/' points to the necessary information.
        # Examples:
        # $ ls -alh /sys/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0/block/sda
        # lrwxrwxrwx  1 root root    0 Feb  6 08:26 device -> ../../../2:0:1:0
        # $ ls -alh /sys/devices/pci0000:00/0000:00:02.2/0000:02:00.0/host1/scsi_host/host1/port-1:6/end_device-1:6/target1:0:5/1:0:5:0/block/sde
        # lrwxrwxrwx  1 root root    0 Oct  12 10:45 device -> ../../../1:0:5:0
        path = os.path.realpath(os.path.join(
            '/sys', dev_path.strip('/'), 'device/'))
        parts = os.path.basename(path).split(':')
        return cls(*map(int, parts))

    def __str__(self):
        return f'{self.host}:{self.channel}:{self.target}:{self.lun}'


class StorageDevicePlugin(openmediavault.device.IStorageDevicePlugin):
    def match(self, device_file):
        device_name = re.sub(r'^/dev/', '', device_file)
        return re.match(r'^sd[a-z]+[0-9]*$', device_name) is not None

    def from_device_file(self, device_file):
        sd = StorageDevice(device_file)
        if sd.host_driver == 'arcmsr':
            # Areca RAID controller
            return StorageDeviceARCMSR(device_file)
        if sd.host_driver == 'hpsa':
            # Logical drives from HP Smart Array RAID controllers
            # are accessed via the SCSI disk driver, so we need to
            # identify such drives because these must be handled
            # different in some cases (e.g. S.M.A.R.T.).
            # @see https://linux.die.net/man/4/hpsa
            return StorageDeviceHPSA(device_file)
        return sd


class StorageDevice(openmediavault.device.StorageDevice):
    """
    Implements the storage device for SCSI disk devices.
    """

    @property
    def parent(self):
        # /dev/sda1 -> /dev/sda
        parent_device_file = re.sub(r'(\d+)$', '', self.canonical_device_file)
        if parent_device_file != self.canonical_device_file:
            return self.__class__(parent_device_file)
        return None

    @property
    def description(self):
        return 'SCSI Disk'

    @property
    def smart_device_type(self):
        if self.is_usb:
            # Identify by ID_VENDOR_ID and ID_MODEL_ID.
            if self.has_udev_property('ID_VENDOR_ID') and self.has_udev_property('ID_MODEL_ID'):
                vendor_id = self.udev_property('ID_VENDOR_ID')
                model_id = self.udev_property('ID_MODEL_ID')
                # ASMedia ASM2362 USB to NVMe bridge
                # https://www.argon40.com/products/argon-one-m-2-expansion-board-nvme
                # https://www.smartmontools.org/ticket/1221
                if '174c' == vendor_id and '2362' == model_id:
                    return 'sntasmedia'
            # Identify by ID_MODEL_ID or `/sys/block/<XXX>/device/model`.
            model_map = {
                'TR-004 DISK00': 'jmb39x-q,0',  # QNAP TR-004
                'TR-004 DISK01': 'jmb39x-q,1',
                'TR-004 DISK02': 'jmb39x-q,2',
                'TR-004 DISK03': 'jmb39x-q,3'
            }
            return model_map.get(self.model.strip(), 'sat')
        return ''

    @cached_property
    def hctl(self) -> HCTL:
        """
        Get the SCSI address of the device.

        The following fields are included:
        - HBA number [host]
        - Channel on the HBA [channel]
        - SCSI target ID [target]
        - LUN [lun]

        See https://tldp.org/HOWTO/SCSI-2.4-HOWTO/scsiaddr.html

        :return: Returns a named tuple with the fields host, bus, target
          and lun.
        """
        return HCTL.from_dev_path(self.udev_property('DEVPATH'))

    @cached_property
    def host_driver(self) -> Optional[str]:
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
        # Try to get the driver via /sys/class/scsi_host/hostN/proc_name.
        # 'proc_name' is the "name of proc directory" of a driver, if
        # the driver maintained one.
        proc_name_path = '/sys/class/scsi_host/host{}/proc_name'.format(
            self.hctl.host)
        try:
            with open(proc_name_path, 'r') as f:
                return f.readline().strip()
        except (IOError, FileNotFoundError):
            pass
        return None

    @cached_property
    def _is_sas(self) -> bool:
        # drwxr-xr-x 8 root root    0 Jul  1 17:15 .
        # drwxr-xr-x 4 root root    0 Jul  1 17:15 ..
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 access_state
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 blacklist
        # drwxr-xr-x 3 root root    0 Jul  1 17:15 block
        # drwxr-xr-x 3 root root    0 Jul  1 17:15 bsg
        # --w------- 1 root root 4.0K Jul  4 08:49 delete
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 device_blocked
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 device_busy
        # -rw-r--r-- 1 root root 4.0K Jul  4 08:49 dh_state
        # lrwxrwxrwx 1 root root    0 Jul  4 08:49 driver -> ../../../../../../../../../..                                                                                                  /bus/scsi/drivers/sd
        # ...
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 modalias
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 model
        # ...
        # --w------- 1 root root 4.0K Jul  4 08:49 rescan
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 rev
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 sas_address
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 sas_device_handle
        # -rw-r--r-- 1 root root 4.0K Jul  4 08:49 sas_ncq_prio_enable
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 sas_ncq_prio_supported
        # drwxr-xr-x 3 root root    0 Jul  1 17:15 scsi_device
        # drwxr-xr-x 3 root root    0 Jul  1 17:15 scsi_disk
        # drwxr-xr-x 3 root root    0 Jul  1 17:15 scsi_generic
        # -r--r--r-- 1 root root 4.0K Jul  4 08:49 scsi_level
        file = '/sys/block/{}/device/sas_address'.format(
            self.device_name(True))
        if os.path.exists(file):
            return True
        # E: ID_PATH=pci-0000:00:10.0-sas-exp0x5001438035ab31bd-phy18-lun-0
        if self.has_udev_property('ID_PATH'):
            value = self.udev_property('ID_PATH')
            return re.match(r'^.+-sas-.+$', value) is not None
        return False

    @cached_property
    def is_hot_pluggable(self) -> bool:
        return self._is_sas or super().is_hot_pluggable


class StorageDeviceARCMSR(StorageDevice):
    """
    Implements the storage device for Areca RAID controllers using
    the arcmsr driver.
    """

    @property
    def serial(self):
        if self.has_udev_property('SCSI_IDENT_SERIAL'):
            return self.udev_property('SCSI_IDENT_SERIAL')
        return super().serial

    @property
    def is_raid(self):
        return True


class StorageDeviceHPSA(StorageDevice):
    """
    Implements the storage device for HP Smart Array RAID controllers
    using the hpsa driver.
    """

    @property
    def is_raid(self):
        return self.raid_level.startswith('RAID')

    @property
    def smart_device_type(self):
        # Note, we need to distinguish between RAID and HBA mode here.
        if self.is_raid:
            return super().smart_device_type
        # $ hpssacli ctrl slot=0 pd all show detail
        # ...
        # physicaldrive 1I:1:5
        # Port: 1I
        # Box: 1
        # Bay: 5
        # Status: OK
        # Disk Name: /dev/sde
        # ...

        # The drive bay is encoded in the devpath of the device via the
        # SCSI address (HCTL), e.g.
        # /devices/pci0000:00/0000:00:02.2/0000:02:00.0/host1/scsi_host/host1/port-1:6/end_device-1:6/target1:0:5/1:0:5:0/block/sde

        # Return 'cciss,N'. The non-negative integer N (in the
        # range from 0 to 15 inclusive) denotes which disk on the
        # controller is monitored.
        return 'cciss,{}'.format(self.hctl.target - 1)

    @property
    def raid_level(self) -> str:
        """
        Get the RAID level of the device. If it is a logical device,
        then 'N/A' will be returned, otherwise 'RAID 0', 'RAID 1(+0)'
        or 'RAID <N>'.

        `https://www.kernel.org/doc/html/latest/scsi/hpsa.html#hpsa-specific-disk-attributes`
        `https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L654`
        `https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L694`
        `https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L702`
        """
        file_path = '/sys/class/scsi_disk/{}/device/raid_level'.format(
            str(self.hctl))
        try:
            with open(file_path, 'r') as f:
                return f.readline().strip()
        except (IOError, FileNotFoundError):
            pass
        return 'N/A'
