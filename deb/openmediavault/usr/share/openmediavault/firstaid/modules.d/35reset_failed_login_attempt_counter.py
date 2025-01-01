#!/usr/bin/env python3
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
import glob
import os
import struct
import sys
from typing import List, NamedTuple

import dialog
import openmediavault.firstaid
import openmediavault.procutils


class Tally(NamedTuple):
    source: str
    reserved: int
    status: int
    time: int

    # https://github.com/linux-pam/linux-pam/blob/master/modules/pam_faillock/faillock.h
    FORMAT = '52sHHQ'
    STATUS_VALID = 0x1  # the tally file entry is valid
    STATUS_RHOST = 0x2  # the source is rhost
    STATUS_TTY = 0x4  # the source is tty

    @property
    def valid(self):
        return self.status & self.STATUS_VALID


class TallyFile:
    def __init__(self, path):
        self.path: str = path
        self.records: List[Tally] = []

    @property
    def name(self) -> str:
        """
        Get the name of the user.
        :return: Returns the name of the user.
        """
        return os.path.basename(self.path)

    def read(self) -> None:
        with open(self.path, 'rb') as f:
            while True:
                data = f.read(struct.calcsize(Tally.FORMAT))
                if not data:
                    break
                record = Tally._make(struct.unpack(Tally.FORMAT, data))
                self.records.append(record)

    @property
    def valid_records(self) -> List[Tally]:
        return [r for r in self.records if r.valid]


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Reset failed login attempt counter"

    def execute(self):
        choices = []
        for path in glob.glob('/var/run/faillock/*'):
            f = TallyFile(path)
            f.read()
            failed_attempts = len(f.valid_records)
            if failed_attempts:
                choices.append([f.name, f'{failed_attempts} failed attempts'])
        if not choices:
            print("No locked/banned users or candidates exists.")
            return 0
        d = dialog.Dialog(dialog="dialog")
        (code, tag) = d.menu(
            "Please select an user to reset the failed login attempt "
            "counter. The maximum number of unsuccessful attempts does "
            "not mean that the user has been locked.",
            backtitle=self.description,
            clear=True,
            height=15,
            width=68,
            menu_height=5,
            choices=choices,
        )
        if code in (d.CANCEL, d.ESC):
            return 0
        username = tag
        print(
            "Reset failed login attempt counter for user '{}'.".format(
                username)
        )
        openmediavault.procutils.check_call(
            ["faillock", "--reset", "--user", username]
        )
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
