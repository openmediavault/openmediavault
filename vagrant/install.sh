#!/bin/sh
#
# This file is part of openmediavault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2021 Volker Theile
#
# openmediavault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# openmediavault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with openmediavault. If not, see <http://www.gnu.org/licenses/>.

set -e

# Append user 'vagrant' to group 'ssh', otherwise the user is not allowed
# to log in via SSH.
usermod --groups ssh --append vagrant

export LANG=C.UTF-8
export DEBIAN_FRONTEND=noninteractive
export APT_LISTCHANGES_FRONTEND=none

# Install the openmediavault keyring manually.
apt-get install --yes gnupg
wget -O "/etc/apt/trusted.gpg.d/openmediavault-archive-keyring.asc" https://packages.openmediavault.org/public/archive.key
apt-key add "/etc/apt/trusted.gpg.d/openmediavault-archive-keyring.asc"

# Install openmediavault.
cat <<EOF >> /etc/apt/sources.list.d/openmediavault.list
deb http://packages.openmediavault.org/public usul main
# deb http://downloads.sourceforge.net/project/openmediavault/packages usul main
## Uncomment the following line to add software from the proposed repository.
# deb http://packages.openmediavault.org/public usul-proposed main
# deb http://downloads.sourceforge.net/project/openmediavault/packages usul-proposed main
## This software is not part of OpenMediaVault, but is offered by third-party
## developers as a service to OpenMediaVault users.
# deb http://packages.openmediavault.org/public usul partner
# deb http://downloads.sourceforge.net/project/openmediavault/packages usul partner
EOF
apt-get update
apt-get --yes --auto-remove --show-upgraded \
	--allow-downgrades --allow-change-held-packages \
	--no-install-recommends \
	--option Dpkg::Options::="--force-confdef" \
	--option DPkg::Options::="--force-confold" \
	install openmediavault-keyring openmediavault

# Populate the database.
omv-confdbadm populate

# Display the login information.
cat /etc/issue
