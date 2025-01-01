# -*- mode: makefile; coding: utf-8 -*-
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

NUM_PROCESSORS := $(shell nproc)

OMV_PACKAGE := $(shell pwd | sed 's|.*/||')
OMV_POT_DIR := $(CURDIR)/usr/share/openmediavault/locale
OMV_POT_FILE := $(OMV_PACKAGE).pot
OMV_TRANSIFEX_PROJECT_SLUG := openmediavault
OMV_XGETTEXT_ARGS := --keyword=_ --output-dir=$(OMV_POT_DIR) \
	--output=$(OMV_POT_FILE) --force-po --no-location --no-wrap \
	--sort-output --package-name=$(OMV_PACKAGE) --from-code=UTF-8 \
	--join-existing

override_dh_builddeb:
	dh_builddeb -- -Zxz

omv_tx_status:
	cd "$(CURDIR)/../"; tx status \
		$(OMV_TRANSIFEX_PROJECT_SLUG).$(OMV_PACKAGE)

omv_tx_pull_po:
	cd "$(CURDIR)/../"; tx pull --all --force \
		$(OMV_TRANSIFEX_PROJECT_SLUG).$(OMV_PACKAGE)

omv_tx_push_pot:
	cd "$(CURDIR)/../"; tx push --source \
		$(OMV_TRANSIFEX_PROJECT_SLUG).$(OMV_PACKAGE)

omv_build_pot:
	dh_testdir
	dh_clean
	echo "Building PO template file ..." >&2
	mkdir -p $(OMV_POT_DIR)
	cat /dev/null > $(OMV_POT_DIR)/$(OMV_POT_FILE)
	# Extract strings from Angular project.
	if [ -e "$(CURDIR)/workbench/package.json" ]; then \
		cd $(CURDIR)/workbench && npm ci && \
			npm run i18n:extract && \
			mv src/assets/i18n/openmediavault-workbench.pot $(OMV_POT_DIR)/$(OMV_POT_FILE); \
	fi
	# Extract strings from YAML files.
	find "$(CURDIR)/usr/share/openmediavault/workbench/" \
		\( -iname *.yaml \) -type f -print0 | \
		xargs -0r xgettext $(OMV_XGETTEXT_ARGS) -
	# Extract strings from PHP files.
	find "$(CURDIR)/usr/share/php/" "$(CURDIR)/usr/share/openmediavault/" \
		\( -iname *.php -o -iname *.inc \) -type f -print0 | \
		xargs -0r xgettext $(OMV_XGETTEXT_ARGS) -
	# Remove '#, c-format' comments, otherwise manually upload of translation
	# files confuses Transifex.
	sed --in-place '/^#, c-format/d' $(OMV_POT_DIR)/$(OMV_POT_FILE)

omv_clean_scm:
	dh_testdir
	echo "Removing SCM files ..." >&2
	find "$(CURDIR)/debian/$(OMV_PACKAGE)/" \( -name .svn -o -name .git \) \
		-type d -print0 -prune | xargs -0r rm -rf

omv_build_doc: debian/doxygen.conf
	mkdir -p debian/doxygen
	doxygen $<

omv_beautify_py:
	autopep8 --max-line-length 79 --in-place --recursive --verbose .
	isort .

omv_lint_py:
	find "$(CURDIR)/" \( -iname *.py \) -type f -print0 | xargs -0r \
		pylint --rcfile="$(CURDIR)/../.pylintrc" --jobs=$(NUM_PROCESSORS)

source: clean
	dpkg-buildpackage -S -us -uc

.PHONY: omv_tx_status omv_tx_pull_po omv_tx_push_pot
.PHONY: omv_build_pot omv_build_doc omv_clean_scm
.PHONY: omv_beautify_py omv_lint_py source
