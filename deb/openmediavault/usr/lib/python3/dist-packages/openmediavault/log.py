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
__all__ = ["info", "warning", "error", "debug"]

import sys
import syslog


def _log(priority, msg, args, verbose=True):
    tag = {
        syslog.LOG_INFO: "INFO",
        syslog.LOG_WARNING: "WARNING",
        syslog.LOG_ERR: "ERROR",
        syslog.LOG_DEBUG: "DEBUG",
    }
    msg = msg % args if args else msg
    if verbose:
        sys.stderr.write("{}: {}\n".format(tag[priority], msg))
    syslog.openlog(facility=syslog.LOG_SYSLOG)
    syslog.syslog(priority, msg)
    syslog.closelog()


def info(msg, *args, **kwargs):
    """
    Log 'msg % args' to STDERR and syslog with priority 'INFO'.
    To do not write to STDERR, use the keyword argument verbose, e.g.

    info("This is a %s", "test", verbose=False)
    """
    _log(syslog.LOG_INFO, msg, args, **kwargs)


def warning(msg, *args, **kwargs):
    """
    Log 'msg % args' to STDERR and syslog with priority 'WARNING'.
    To do not write to STDERR, use the keyword argument verbose, e.g.

    warning("This is a %s", "test", verbose=False)
    """
    _log(syslog.LOG_WARNING, msg, args, **kwargs)


def error(msg, *args, **kwargs):
    """
    Log 'msg % args' to STDERR and syslog with priority 'ERR'.
    To do not write to STDERR, use the keyword argument verbose, e.g.

    error("This is a %s", "test", verbose=False)
    """
    _log(syslog.LOG_ERR, msg, args, **kwargs)


def debug(msg, *args, **kwargs):
    """
    Log 'msg % args' to STDERR and syslog with priority 'DEBUG'.
    To do not write to STDERR, use the keyword argument verbose, e.g.

    debug("This is a %s", "test", verbose=False)
    """
    _log(syslog.LOG_DEBUG, msg, args, **kwargs)
