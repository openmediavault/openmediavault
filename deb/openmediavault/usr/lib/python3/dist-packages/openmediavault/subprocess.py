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
"""
This module allows you to spawn processes, connect to their
input/output/error pipes, and obtain their return codes.

It redirects the function calls to the functions provided by
the subprocess module. The only difference is that this module
takes care that the 'env' keyword argument is set and the
'LANG' environment variable is set to 'C'. This is because
we always want to have not localized command output to do not
get error message in foreign languages that the maintainers
do not understand.
"""
__all__ = ["Popen", "call", "check_call", "check_output"]

import os
import subprocess  # lgtm[py/import-own-module]


def _modify_kwargs(kwargs):
    """
    Append 'LANG="C"' to the 'env' keyword argument because we
    always want untranslated command line output.
    """
    # Append the env keyword if it does not exist.
    if "env" not in kwargs:
        kwargs["env"] = dict(os.environ, LANG="C.UTF-8")
    else:
        kwargs["env"].update({"LANG": "C.UTF-8"})


def call(*popenargs, **kwargs):
    _modify_kwargs(kwargs)
    return subprocess.call(*popenargs, **kwargs)


def check_call(*popenargs, **kwargs):
    _modify_kwargs(kwargs)
    return subprocess.check_call(*popenargs, **kwargs)


def check_output(*popenargs, **kwargs):
    _modify_kwargs(kwargs)
    return subprocess.check_output(*popenargs, **kwargs)


class Popen(subprocess.Popen):
    def __init__(self, *popenargs, **kwargs):
        _modify_kwargs(kwargs)
        super().__init__(*popenargs, **kwargs)  # lgtm[py/super-in-old-style]
