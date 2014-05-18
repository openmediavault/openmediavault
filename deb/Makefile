# -*- mode: makefile; coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2014 Volker Theile
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

OMV_SUBDIRS := $(wildcard openmediavault*)

omv_pull_po_all:
	for dir in $(OMV_SUBDIRS); do \
		cd $$dir && fakeroot debian/rules omv_pull_po; \
	done

omv_push_pot_all:
	for dir in $(OMV_SUBDIRS); do \
		cd $$dir && fakeroot debian/rules omv_push_pot; \
	done

omv_build_pot_all:
	for dir in $(OMV_SUBDIRS); do \
		cd $$dir && fakeroot debian/rules omv_build_pot; \
	done

.PHONY: omv_pull_po_all omv_push_pot_all omv_build_pot_all
