# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <votdev@gmx.de>
# @copyright Copyright (c) 2009-2023 Volker Theile
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

# Install before usage:
# apt-get install pbuilder qemubuilder qemu-user-static

# This packages need to be installed to fix a pbuilder issue.
FIX_DEBOOTSTRAP_PKG_INSTALL=perl-openssl-defaults

DISTRIBUTION=bullseye
OMV_DISTRIBUTION=shaitan
ARCHITECTURE?=amd64
NAME=$(DISTRIBUTION)-$(ARCHITECTURE)
MIRROR="http://deb.debian.org/debian/"
OTHERMIRROR="deb http://packages.openmediavault.org/public $(OMV_DISTRIBUTION) main|deb-src http://packages.openmediavault.org/public $(OMV_DISTRIBUTION) main"
BASEDIR="/var/cache/pbuilder"
BASETGZ="$(BASEDIR)/$(NAME)-base.tgz"
BUILDPLACE="$(BASEDIR)/build/"
BUILDRESULT="$(BASEDIR)/$(NAME)/result/"
APTCACHE="$(BASEDIR)/$(NAME)/aptcache/"
COMPONENTS="main contrib non-free"
DEBOOTSTRAPOPTS_KEYRING=/usr/share/keyrings/debian-archive-keyring.gpg
DEBOOTSTRAPOPTS_INCLUDE=python3,devscripts,fakeroot,debhelper,php-dev,nano,quilt,equivs,gnupg,$(FIX_DEBOOTSTRAP_PKG_INSTALL)
KEYRING=/etc/apt/trusted.gpg.d/openmediavault-archive-keyring.gpg
DSCFILE?=""

$(BUILDPLACE) $(BUILDRESULT) $(APTCACHE):
	sudo mkdir -p $@

install_keyring:
	wget --quiet --output-document=- https://packages.openmediavault.org/public/archive.key | \
		gpg --dearmor | sudo tee $(KEYRING)

build:
	sudo pbuilder build \
		--configfile .pbuilderrc \
		--basetgz $(BASETGZ) \
		--buildresult $(BUILDRESULT) \
		--aptcache $(APTCACHE) \
		$(DSCFILE)

update:
	sudo pbuilder update \
		--basetgz $(BASETGZ)

clean:
	sudo pbuilder clean \
		--buildplace $(BUILDPLACE) \
		--aptcache $(APTCACHE)

create: $(BUILDPLACE) $(BUILDRESULT) $(APTCACHE) install_keyring
	sudo pbuilder create \
		--configfile .pbuilderrc \
		--distribution $(DISTRIBUTION) \
		--architecture $(ARCHITECTURE) \
		--basetgz $(BASETGZ) \
		--mirror $(MIRROR) \
		--othermirror $(OTHERMIRROR) \
		--buildplace $(BUILDPLACE) \
		--buildresult $(BUILDRESULT) \
		--aptcache $(APTCACHE) \
		--components $(COMPONENTS) \
		--debootstrap qemu-debootstrap \
		--debootstrapopts --verbose \
		--debootstrapopts --arch=$(ARCHITECTURE) \
		--debootstrapopts --keyring=$(DEBOOTSTRAPOPTS_KEYRING) \
		--debootstrapopts --include=$(DEBOOTSTRAPOPTS_INCLUDE) \
		--keyring $(KEYRING)

list:
	find $(BASEDIR) -maxdepth 1 -type f -iname *-base.tgz \
	  -printf "%f\n" | sed 's:^\(.*-.*\)-base.tgz$$:\1:' | sort

login:
	sudo pbuilder login \
		--basetgz $(BASETGZ)

showresult:
	xdg-open $(BUILDRESULT)

.PHONY: build clean create login showresult list
