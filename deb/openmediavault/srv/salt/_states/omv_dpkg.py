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
import os.path
import subprocess

import openmediavault.procutils


def divert_add(name):
    """
    Override a package's version of a file.
    The diverted file is located in `/tmp` to ensure that it has no side
    effects, e.g. otherwise a daily cron job would still be installed in
    `/etc/cron.daily` as `/etc/cron.daily/<FILE>.distrib` and thus also
    executed.
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
        divert_to = os.path.join('/tmp', name.replace(os.path.sep, '_'))
        ret['comment'] = openmediavault.procutils.check_output(
            [
                'dpkg-divert', '--add', '--local', '--no-rename', '--divert',
                divert_to, name
            ],
            text=True
        )
        ret['result'] = True
    except subprocess.CalledProcessError as e:
        ret['comment'] = str(e)
    return ret
