#!/bin/sh
#
# This file is part of openmediavault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
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

# Disable the Debian backport repositories.
sed -i '/-backports/s/^/#/' /etc/apt/sources.list

# Append user 'vagrant' to group 'ssh', otherwise the user is not allowed
# to log in via SSH.
# https://salsa.debian.org/ssh-team/openssh/-/commit/18da782ebe789d0cf107a550e474ba6352e68911
usermod --groups _ssh --append vagrant

# Set default password for the 'root' user. This is helpful if something
# went wrong during the development.
# Note, the 'Permit root login' checkbox must be checked to make this
# working.
yes openmediavault | passwd

export LANG=C.UTF-8
export DEBIAN_FRONTEND=noninteractive
export APT_LISTCHANGES_FRONTEND=none

# Install the openmediavault keyring manually.
apt-get install --yes gnupg
wget --quiet --output-document=- https://packages.openmediavault.org/public/archive.key | \
	gpg --dearmor --yes --output "/usr/share/keyrings/openmediavault-archive-keyring.gpg"

# Install openmediavault.
cat <<EOF >> /etc/apt/sources.list.d/openmediavault.list
deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://packages.openmediavault.org/public sandworm main
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://downloads.sourceforge.net/project/openmediavault/packages sandworm main
## Uncomment the following line to add software from the proposed repository.
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://packages.openmediavault.org/public sandworm-proposed main
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://downloads.sourceforge.net/project/openmediavault/packages sandworm-proposed main
## This software is not part of OpenMediaVault, but is offered by third-party
## developers as a service to OpenMediaVault users.
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://packages.openmediavault.org/public sandworm partner
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://downloads.sourceforge.net/project/openmediavault/packages sandworm partner
EOF
apt-get update
apt-get --yes --auto-remove --show-upgraded \
	--allow-downgrades --allow-change-held-packages \
	--no-install-recommends \
	--option DPkg::Options::="--force-confdef" \
	--option DPkg::Options::="--force-confold" \
	install openmediavault

# Populate the database.
omv-confdbadm populate

# Deploy the /etc/hosts file to ensure the hostname can be resolved
# properly for IPv4 and IPv6. Otherwise building the Salt grains
# (core.fqdns and core.ip_fqdn) will take a very long time.
omv-salt deploy run hosts

# Display the login information.
cat /etc/issue
