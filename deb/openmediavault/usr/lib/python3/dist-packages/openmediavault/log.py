# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2016 Volker Theile
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
__all__ = [ "info", "warning", "error", "debug" ]

import sys
import syslog

def _log(message, priority, verbose):
	tag = {
		syslog.LOG_INFO: "INFO",
		syslog.LOG_WARNING: "WARNING",
		syslog.LOG_ERR: "ERROR",
		syslog.LOG_DEBUG: "DEBUG"
	}
	if verbose:
		sys.stderr.write("{}: {}\n".format(tag[priority], message))
	syslog.openlog(facility=syslog.LOG_SYSLOG)
	syslog.syslog(priority, message)
	syslog.closelog()

def info(message, verbose=True):
	_log(message, syslog.LOG_INFO, verbose)

def warning(message, verbose=True):
	_log(message, syslog.LOG_WARNING, verbose)

def error(message, verbose=True):
	_log(message, syslog.LOG_ERR, verbose)

def debug(message, verbose=True):
	_log(message, syslog.LOG_DEBUG, verbose)
