# -*- mode: makefile; coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2013 Volker Theile
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

OMV_PACKAGE := $(shell pwd | sed 's|.*/||')
OMV_POT_DIR := $(CURDIR)/usr/share/openmediavault/locale
OMV_POT_FILE := $(OMV_PACKAGE).pot
OMV_TRANSIFEX_PROJECT_SLUG := openmediavault

omv_pull_po:
	tx --root="$(CURDIR)/../" pull --all \
	  --resource=$(OMV_TRANSIFEX_PROJECT_SLUG).$(OMV_PACKAGE)

omv_push_pot:
	tx --root="$(CURDIR)/../" push --source \
	  --resource=$(OMV_TRANSIFEX_PROJECT_SLUG).$(OMV_PACKAGE)

omv_build_pot:
	dh_testdir
	dh_clean
	echo "Building PO template file ..." >&2
	mkdir -p $(OMV_POT_DIR)
	find $(CURDIR) \( -iname *.js -o -iname *.php -o -iname *.inc \) \
	  -type f -print0 | xargs -0r xgettext --keyword=_ \
	  --output-dir=$(OMV_POT_DIR) --output=$(OMV_POT_FILE) \
	  --force-po --no-location --no-wrap --sort-output \
	  --package-name=$(OMV_PACKAGE) -
	# Remove '#, c-format' comments, otherwise manuall upload of translation
	# files confuses Transifex.
	sed --in-place '/^#, c-format/d' $(OMV_POT_DIR)/$(OMV_POT_FILE)

omv_clean_scm:
	dh_testdir
	echo "Removing SCM files ..." >&2
	find $(CURDIR)/debian/$(OMV_PACKAGE) \( -name .svn -o -name .git \) \
	  -type d -print0 -prune | xargs -0r rm -rf

omv_build_doc: debian/doxygen.conf
	mkdir -p debian/doxygen
	doxygen $<

.PHONY: omv_pull_po omv_push_pot omv_build_pot omv_clean_scm omv_build_doc
