# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2024 Volker Theile
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
import subprocess

import openmediavault.procutils


def divert_add(name):
    """
    Override a package's version of a file.
    :param name: The name of the file to process.
    :type name: str
    """
    ret = {
        'name': name,
        'changes': {},
        'result': False,
        'comment': ''
    }
    try:
        ret['comment'] = openmediavault.procutils.check_output(
            [
                'dpkg-divert', '--add', '--local', '--no-rename', name
            ],
            text=True
        )
        ret['result'] = True
    except subprocess.CalledProcessError as e:
        ret['comment'] = str(e)
    return ret
