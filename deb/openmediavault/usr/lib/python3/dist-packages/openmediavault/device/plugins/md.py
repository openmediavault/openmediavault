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
import re

import openmediavault.collections
import openmediavault.device
import openmediavault.subprocess


class StorageDevicePlugin(openmediavault.device.IStorageDevicePlugin):
    def match(self, device_file):
        # Examples:
        # - /dev/md2
        # - /dev/md0p1
        device_name = re.sub(r'^/dev/', '', device_file)
        return re.match(r'^md\d+(p\d+)*$', device_name) is not None

    def from_device_file(self, device_file):
        return StorageDevice(device_file)


class StorageDevice(openmediavault.device.StorageDevice):
    """
    Implements the storage device for mdadm devices.
    @see https://en.wikipedia.org/wiki/Mdadm
    """

    def __init__(self, device_file):
        super().__init__(device_file)
        # Init internals.
        self._name = None
        self._uuid = None
        self._level = None
        self._devices = None
        self._num_devices = None
        self._state = None
        self._metadata = None
        self._has_write_intent_bitmap = None
        self._details = None
        self._cached = False

    @property
    def parent(self):
        # /dev/md0p1 -> /dev/md0
        parent_device_file = re.sub(r'(p\d+)$', '', self.canonical_device_file)
        if parent_device_file != self.canonical_device_file:
            return self.__class__(parent_device_file)
        return None

    def _get_data(self):
        if self._cached:
            return

        output = openmediavault.subprocess.check_output(
            ['mdadm', '--detail', '--brief', '--verbose', self.device_file]
        )
        # ARRAY /dev/md0 level=raid5 num-devices=3 metadata=1.2 name=xxxx:0 UUID=a4266bf7:c671b343:c3d6e535:ca455e37
        #    devices=/dev/sdb,/dev/sdc,/dev/sdd
        parts = re.sub(r'\s+', ' ', output.decode()).split(' ')
        for part in parts:
            try:
                key, value = part.split('=')
                setattr(
                    self, '_{}'.format(key.lower().replace('-', '_')), value
                )
            except ValueError:
                pass

        # Get more information.
        output = openmediavault.subprocess.check_output(
            ['mdadm', '--detail', self.device_file]
        )
        # /dev/md0:
        #         Version : 1.2
        #   Creation Time : Tue Dec 25 21:58:20 2012
        #      Raid Level : raid5
        #      Array Size : 207872 (203.03 MiB 212.86 MB)
        #   Used Dev Size : 103936 (101.52 MiB 106.43 MB)
        #    Raid Devices : 3
        #   Total Devices : 3
        #     Persistence : Superblock is persistent
        #
        #   Intent Bitmap : Internal
        #
        #     Update Time : Tue Dec 25 22:31:32 2012
        #           State : active
        #  Active Devices : 3
        # Working Devices : 3
        #  Failed Devices : 0
        #   Spare Devices : 0
        #
        #          Layout : left-symmetric
        #      Chunk Size : 512K
        #
        #            Name : dhcppc2:0  (local to host dhcppc2)
        #            UUID : 9d85a4f6:afff2cb6:b8a5f4dc:75f3cfd3
        #          Events : 37
        #
        #     Number   Major   Minor   RaidDevice State
        #        0       8       16        0      active sync   /dev/sdb
        #        1       8       48        1      active sync   /dev/sdd
        #        2       8       64        2      active sync   /dev/sde
        output = output.decode().split('\n')
        output.pop(0)
        self._details = '\n'.join(output)

        for line in output:
            try:
                key, value = line.split(':')
                key = key.strip().lower()
                value = value.strip()
                if key == 'state':
                    self._state = value
                elif key == 'intent bitmap':
                    self._has_write_intent_bitmap = (
                        True if value == 'Internal' else False
                    )
            except ValueError:
                pass

        self._cached = True

    @property
    def description(self):
        """
        :rtype: str
        """
        return 'MD RAID'

    @property
    def exists(self):
        """
        Checks if the device exists.
        :rtype: bool
        """
        try:
            self._get_data()
        except Exception:
            return False
        return 0 < len(self.uuid)

    @property
    def uuid(self):
        """
        Get the UUID of the array.
        :return: The UUID of the array.
        :rtype: str
        """
        self._get_data()
        return self._uuid

    @property
    def name(self):
        """
        Get the name of the logical volume.
        :return: The logical volume name.
        :rtype: str
        """
        self._get_data()
        return self._name

    @property
    def level(self):
        """
        Get the level of the array.
        :return: The level of the array.
        :rtype: str
        """
        self._get_data()
        return self._level

    @property
    def num_devices(self):
        """
        Get the number of active devices in the array.
        :return: The number of active devices in the array.
        :rtype: int
        """
        self._get_data()
        return int(self._num_devices)

    @property
    def serial(self):
        """
        Get the device serial number.
        :return: The device serial number, otherwise an empty string.
        :rtype: str
        """
        return self.uuid

    @property
    def slaves(self):
        """
        Get the slave devices of this device.
        :return: A list of device files.
        :rtype: list[str]
        """
        return self._devices.split(',')

    @property
    def details(self):
        """
        Get details of this device.
        :return: The details of this device.
        :rtype: str
        """
        self._get_data()
        return self._details

    @property
    def state(self):
        self._get_data()
        with open('/proc/mdstat', 'r') as f:
            content = f.read()
            # Personalities : [linear] [multipath] [raid0] [raid1] [raid6] [raid5] [raid4] [raid10]
            # md1 : active raid5 sde[2] sdg[1] sdf[0]
            #       207872 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
            #       [=========>...........]  resync = 45.0% (47488/103936) finish=0.0min speed=47488K/sec
            #
            # md0 : active (auto-read-only) raid5 sdd[2] sdc[1] sdb[0]
            #       207872 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
            #       	resync=PENDING
            #
            # md0 : active raid5 sdf[4] sde[3] sdd[2] sdc[1] sdb[0]
            #       311808 blocks super 1.2 level 5, 512k chunk, algorithm 2 [5/5] [UUUUU]
            #       [====>................]  reshape = 20.5% (21504/103936) finish=0.0min speed=21504K/sec
            #
            # md0 : active raid1 sdc[2] sdd[0]
            #       2930135360 blocks super 1.2 [2/1] [U_]
            #       [>....................]  recovery = 1.1% (33131904/2930135360) finish=266.5min speed=181134K/sec
            #
            # md0 : inactive sdb[0](S) sdc[2](S) sdd[1](S)
            #       311808 blocks super 1.2
            #
            # md0 : active raid0 sdd[0] sde[1]
            #       311296 blocks super 1.2 512k chunks
            #
            # unused devices: <none>
            matches = re.match(
                r'.*^({}\s*:.*?)^\s*$'.format(self.device_name(True)),
                content,
                flags=re.IGNORECASE | re.MULTILINE | re.DOTALL
            )
            if not matches:
                return False
            mdstat = matches.group(1)
            progress = 'unknown'
            matches = re.match(
                r'^.+(resync|reshape|recovery)\s*=\s*(.+)$',
                mdstat,
                flags=re.IGNORECASE | re.MULTILINE
            )
            if matches:
                progress = matches.group(2).strip()
            result = self._state
            if self._state.lower() in ['resyncing', 'recovering']:
                if progress.lower() == 'pending':
                    # The state is already in proper form.
                    # # cat /proc/mdstat
                    # Personalities : [raid6] [raid5] [raid4]
                    # md0 : active (auto-read-only) raid5 sda[0] sdd[3](S) sdc[2] sdb[1]
                    #       2095104 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
                    #       	resync=PENDING
                    #
                    # # mdadm --state
                    # ...
                    # Update Time : Fri Dec  2 15:32:42 2016
                    # State : clean, resyncing (PENDING)
                    # Active Devices : 3
                    # Working Devices : 4
                    # Failed Devices : 0
                    # Spare Devices : 1
                    # ...
                    pass
                else:
                    result = '{} ({})'.format(result, progress)
            elif mdstat == 'resync':
                result = '{}, resyncing ({})'.format(result, progress)
            return result

    @property
    def has_write_intent_bitmap(self):
        """
        Check whether the RAID device has write-intent bitmap enabled.
        Note, only 'internal' bitmaps are detected and reported.
        @see https://raid.wiki.kernel.org/index.php/Write-intent_bitmap
        :return: Returns ``True`` if write-intent bitmap is enabled,
            otherwise ``False``.
        :rtype: bool
        """
        self._get_data()
        return self._has_write_intent_bitmap

    @property
    def is_raid(self):
        return True
