# -*- mode: makefile; coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
PHP_BOOTSTRAP_FILE=bootstrap.php
PHP_FILES=$(wildcard test_*.php)
PHP_INCLUDE_PATH=$(CURDIR)/../../../php:/usr/share/php

test:
	@set -e; \
	for file in $(PHP_FILES); do \
		echo ">>> Processing $$file ..."; \
		phpunit --include-path "$(PHP_INCLUDE_PATH)" --bootstrap "$(PHP_BOOTSTRAP_FILE)" $$file; \
		echo; \
	done

install:
	echo "Installing required packages ..."
	sudo apt-get install -y uuid phpunit

.PHONY: test install
